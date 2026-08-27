#!/usr/bin/env bash
set -euo pipefail

# ============================================================================
# Courier404 Multi-Agent Bus
#
# Shared coordination bus for:
#   main orchestrator
#   agent/w1
#   agent/w2
#   agent/w3
#
# Beads remains the authoritative task tracker.
# This bus transports assignments/results and tracks worker liveness only.
#
# IMPORTANT:
#   - Orchestrator owns Beads and integration.
#   - Workers never modify Beads.
#   - Workers never merge branches.
#   - Workers never run full UE verification.
# ============================================================================

readonly SCRIPT_NAME="$(basename "$0")"

readonly COMMON="$(git rev-parse --path-format=absolute --git-common-dir)"
readonly BUS="$COMMON/courier-agent-bus"
readonly LOCK="$BUS/.lock"

readonly DEFAULT_WAIT_TIMEOUT="${AGENT_BUS_WAIT_TIMEOUT:-60}"
readonly DEFAULT_WAIT_INTERVAL="${AGENT_BUS_WAIT_INTERVAL:-2}"
readonly DEFAULT_STALE_SECONDS="${AGENT_BUS_STALE_SECONDS:-180}"

mkdir -p "$BUS"/{w1,w2,w3}
touch "$LOCK"


# ============================================================================
# Utilities
# ============================================================================

die() {
    echo "agent-bus: $*" >&2
    exit 2
}

now_epoch() {
    date +%s
}

require_uint() {
    [[ "${1:-}" =~ ^[0-9]+$ ]] || die "expected unsigned integer, got: ${1:-<empty>}"
}

validate_worker() {
    case "${1:-}" in
        w1|w2|w3) ;;
        *) die "worker must be w1, w2, or w3" ;;
    esac
}

worker_dir() {
    validate_worker "${1:-}"
    printf '%s/%s\n' "$BUS" "$1"
}

atomic_write() {
    local path="$1"
    shift

    local tmp="${path}.tmp.$$"

    printf '%s\n' "$*" > "$tmp"
    mv -f "$tmp" "$path"
}

read_status() {
    local w="$1"
    local d
    d="$(worker_dir "$w")"

    if [[ -f "$d/status" ]]; then
        cat "$d/status"
    else
        echo "idle"
    fi
}

touch_heartbeat() {
    local w="$1"
    local d
    d="$(worker_dir "$w")"

    atomic_write "$d/heartbeat" "$(now_epoch)"
}

task_issue() {
    local d="$1"

    [[ -f "$d/task" ]] || return 1

    sed -n 's/^ISSUE=//p' "$d/task" | head -n 1
}

result_issue() {
    local d="$1"

    [[ -f "$d/result" ]] || return 1

    sed -n 's/^ISSUE=//p' "$d/result" | head -n 1
}

validate_issue_id() {
    local issue="${1:-}"

    [[ -n "$issue" ]] || die "issue id is required"

    # Prevent the old failure mode where the entire prompt was stuffed into ISSUE.
    [[ "$issue" != *" "* ]] ||
        die "ISSUE must contain only the Beads ID, not the task prompt: $issue"

    [[ "$issue" != *$'\n'* ]] ||
        die "ISSUE must be a single line"

    [[ "$issue" =~ ^[A-Za-z0-9._-]+$ ]] ||
        die "invalid issue id: $issue"
}

with_lock() {
    flock "$LOCK" "$@"
}


# ============================================================================
# Initialization
# ============================================================================

init_bus() {
    local w d

    for w in w1 w2 w3; do
        d="$(worker_dir "$w")"
        mkdir -p "$d"

        if [[ ! -f "$d/status" ]]; then
            atomic_write "$d/status" "idle"
        fi

        if [[ ! -f "$d/heartbeat" ]]; then
            atomic_write "$d/heartbeat" "0"
        fi
    done

    echo "Agent bus: $BUS"
}


# ============================================================================
# Assignment lifecycle
# ============================================================================

assign_task() {
    local w="${1:-}"
    local issue="${2:-}"

    [[ $# -ge 3 ]] || die "usage: $SCRIPT_NAME assign <worker> <ISSUE_ID> \"<prompt>\""

    shift 2

    local prompt="$*"
    local d

    validate_worker "$w"
    validate_issue_id "$issue"

    [[ -n "$prompt" ]] || die "assignment prompt must not be empty"

    d="$(worker_dir "$w")"

    (
        flock -x 9

        local status
        status="$(read_status "$w")"

        case "$status" in
            idle)
                ;;
            *)
                die "$w is not idle: $status"
                ;;
        esac

        rm -f "$d/result"

        local tmp="$d/task.tmp.$$"

        {
            printf 'ISSUE=%s\n' "$issue"
            printf 'PROMPT=%s\n' "$prompt"
        } > "$tmp"

        mv -f "$tmp" "$d/task"

        atomic_write "$d/status" "assigned:$issue"

    ) 9>"$LOCK"

    echo "$w <- $issue"
}

poll_task() {
    local w="${1:-}"
    local d

    validate_worker "$w"
    d="$(worker_dir "$w")"

    touch_heartbeat "$w"

    if [[ -f "$d/task" ]]; then
        cat "$d/task"
    else
        echo "NO_TASK"
    fi

    # Always success.
    # NO_TASK is normal and must not look like a shell failure to Kilo.
    return 0
}

wait_task() {
    local w="${1:-}"
    local timeout="${2:-$DEFAULT_WAIT_TIMEOUT}"
    local interval="${3:-$DEFAULT_WAIT_INTERVAL}"

    validate_worker "$w"
    require_uint "$timeout"
    require_uint "$interval"

    local d
    d="$(worker_dir "$w")"

    local started now elapsed
    started="$(now_epoch)"

    while :; do
        touch_heartbeat "$w"

        if [[ -f "$d/task" ]]; then
            cat "$d/task"
            return 0
        fi

        now="$(now_epoch)"
        elapsed=$(( now - started ))

        if (( elapsed >= timeout )); then
            echo "NO_TASK"
            return 0
        fi

        sleep "$interval"
    done
}

start_task() {
    local w="${1:-}"
    local issue="${2:-}"

    validate_worker "$w"
    validate_issue_id "$issue"

    local d
    d="$(worker_dir "$w")"

    (
        flock -x 9

        [[ -f "$d/task" ]] ||
            die "$w has no assigned task"

        local expected status
        expected="$(task_issue "$d")"
        status="$(read_status "$w")"

        [[ "$expected" == "$issue" ]] ||
            die "$w task mismatch: assigned=$expected requested=$issue"

        case "$status" in
            "assigned:$issue"|"working:$issue")
                ;;
            *)
                die "$w cannot start $issue from state: $status"
                ;;
        esac

        atomic_write "$d/status" "working:$issue"
        atomic_write "$d/heartbeat" "$(now_epoch)"

    ) 9>"$LOCK"

    echo "$w started $issue"
}

done_task() {
    local w="${1:-}"
    local issue="${2:-}"
    local commit="${3:-}"

    [[ $# -ge 4 ]] ||
        die "usage: $SCRIPT_NAME done <worker> <ISSUE_ID> <COMMIT> \"<evidence>\""

    shift 3

    local evidence="$*"

    validate_worker "$w"
    validate_issue_id "$issue"

    [[ -n "$commit" ]] || die "commit hash is required"
    [[ -n "$evidence" ]] || die "completion evidence is required"

    local d
    d="$(worker_dir "$w")"

    (
        flock -x 9

        local expected status
        expected="$(task_issue "$d" 2>/dev/null || true)"
        status="$(read_status "$w")"

        [[ "$expected" == "$issue" ]] ||
            die "$w task mismatch: assigned=${expected:-none} completed=$issue"

        case "$status" in
            "working:$issue"|"assigned:$issue")
                ;;
            *)
                die "$w cannot complete $issue from state: $status"
                ;;
        esac

        local tmp="$d/result.tmp.$$"

        {
            printf 'STATE=done\n'
            printf 'WORKER=%s\n' "$w"
            printf 'ISSUE=%s\n' "$issue"
            printf 'COMMIT=%s\n' "$commit"
            printf 'TIMESTAMP=%s\n' "$(now_epoch)"
            printf 'EVIDENCE=%s\n' "$evidence"
        } > "$tmp"

        mv -f "$tmp" "$d/result"

        atomic_write "$d/status" "done:$issue"
        atomic_write "$d/heartbeat" "$(now_epoch)"

    ) 9>"$LOCK"

    echo "$w done $issue @ $commit"
}

fail_task() {
    local w="${1:-}"
    local issue="${2:-}"

    [[ $# -ge 3 ]] ||
        die "usage: $SCRIPT_NAME fail <worker> <ISSUE_ID> \"<reason>\""

    shift 2

    local reason="$*"

    validate_worker "$w"
    validate_issue_id "$issue"

    [[ -n "$reason" ]] || die "failure reason is required"

    local d
    d="$(worker_dir "$w")"

    (
        flock -x 9

        local expected status
        expected="$(task_issue "$d" 2>/dev/null || true)"
        status="$(read_status "$w")"

        [[ "$expected" == "$issue" ]] ||
            die "$w task mismatch: assigned=${expected:-none} failed=$issue"

        case "$status" in
            "working:$issue"|"assigned:$issue")
                ;;
            *)
                die "$w cannot fail $issue from state: $status"
                ;;
        esac

        local tmp="$d/result.tmp.$$"

        {
            printf 'STATE=failed\n'
            printf 'WORKER=%s\n' "$w"
            printf 'ISSUE=%s\n' "$issue"
            printf 'TIMESTAMP=%s\n' "$(now_epoch)"
            printf 'REASON=%s\n' "$reason"
        } > "$tmp"

        mv -f "$tmp" "$d/result"

        atomic_write "$d/status" "failed:$issue"
        atomic_write "$d/heartbeat" "$(now_epoch)"

    ) 9>"$LOCK"

    echo "$w failed $issue"
}


# ============================================================================
# Results
# ============================================================================

show_results() {
    local w d found=0

    for w in w1 w2 w3; do
        d="$(worker_dir "$w")"

        if [[ -f "$d/result" ]]; then
            echo "===== $w ====="
            cat "$d/result"
            echo
            found=1
        fi
    done

    if (( found == 0 )); then
        echo "NO_RESULTS"
    fi
}

wait_result() {
    local timeout="${1:-$DEFAULT_WAIT_TIMEOUT}"
    local interval="${2:-$DEFAULT_WAIT_INTERVAL}"

    require_uint "$timeout"
    require_uint "$interval"

    local started now elapsed w d
    started="$(now_epoch)"

    while :; do
        for w in w1 w2 w3; do
            d="$(worker_dir "$w")"

            if [[ -f "$d/result" ]]; then
                echo "WORKER_RESULT=$w"
                cat "$d/result"
                return 0
            fi
        done

        now="$(now_epoch)"
        elapsed=$(( now - started ))

        if (( elapsed >= timeout )); then
            echo "NO_RESULT"
            return 0
        fi

        sleep "$interval"
    done
}


# ============================================================================
# ACK / recycle
# ============================================================================

ack_worker() {
    local w="${1:-}"
    local expected_issue="${2:-}"

    validate_worker "$w"

    local d
    d="$(worker_dir "$w")"

    (
        flock -x 9

        local status actual_issue
        status="$(read_status "$w")"

        case "$status" in
            done:*|failed:*)
                ;;
            *)
                die "$w cannot be acked from state: $status"
                ;;
        esac

        actual_issue="$(result_issue "$d" 2>/dev/null || true)"

        if [[ -n "$expected_issue" ]]; then
            validate_issue_id "$expected_issue"

            [[ "$actual_issue" == "$expected_issue" ]] ||
                die "$w ack mismatch: result=${actual_issue:-none} requested=$expected_issue"
        fi

        rm -f "$d/task" "$d/result"

        atomic_write "$d/status" "idle"
        atomic_write "$d/heartbeat" "$(now_epoch)"

    ) 9>"$LOCK"

    echo "$w acknowledged -> idle"
}


# ============================================================================
# Worker branch synchronization
#
# Expected lifecycle:
#
#   worker commit
#   -> orchestrator merges worker into main
#   -> orchestrator verifies
#   -> worker commit is now ancestor of main
#   -> sync worker branch using --ff-only
#   -> ack worker
#
# sync NEVER hard-resets a worker branch.
# ============================================================================

find_worker_worktree() {
    local w="$1"
    local wanted="refs/heads/agent/$w"

    git worktree list --porcelain |
        awk -v wanted="$wanted" '
            /^worktree / {
                path = substr($0, 10)
            }

            /^branch / {
                branch = substr($0, 8)

                if (branch == wanted) {
                    print path
                    exit
                }
            }
        '
}

sync_worker() {
    local w="${1:-}"

    validate_worker "$w"

    local path
    path="$(find_worker_worktree "$w")"

    [[ -n "$path" ]] ||
        die "could not locate worktree for agent/$w"

    [[ -d "$path" ]] ||
        die "worker worktree does not exist: $path"

    local branch
    branch="$(git -C "$path" branch --show-current)"

    [[ "$branch" == "agent/$w" ]] ||
        die "$w worktree is on unexpected branch: $branch"

    if [[ -n "$(git -C "$path" status --porcelain)" ]]; then
        echo "agent-bus: refusing to sync dirty worktree $path" >&2
        git -C "$path" status --short >&2
        exit 3
    fi

    # Ensure main exists.
    git show-ref --verify --quiet refs/heads/main ||
        die "local main branch does not exist"

    # Worker branch must be able to fast-forward to main.
    if ! git merge-base --is-ancestor "agent/$w" main; then
        die "agent/$w is not an ancestor of main; refusing non-fast-forward sync"
    fi

    git -C "$path" merge --ff-only main

    echo "$w synchronized to $(git -C "$path" rev-parse --short HEAD)"
}

recycle_worker() {
    local w="${1:-}"
    local issue="${2:-}"

    validate_worker "$w"

    # First make the branch current with main.
    sync_worker "$w"

    # Only after successful synchronization clear the completed bus state.
    if [[ -n "$issue" ]]; then
        ack_worker "$w" "$issue"
    else
        ack_worker "$w"
    fi

    echo "$w recycled and ready"
}


# ============================================================================
# Heartbeats / liveness
# ============================================================================

heartbeat() {
    local w="${1:-}"

    validate_worker "$w"
    touch_heartbeat "$w"

    echo "$w heartbeat $(now_epoch)"
}

health() {
    local stale_seconds="${1:-$DEFAULT_STALE_SECONDS}"

    require_uint "$stale_seconds"

    local now
    now="$(now_epoch)"

    local w d status hb age health_state

    for w in w1 w2 w3; do
        d="$(worker_dir "$w")"
        status="$(read_status "$w")"
        hb="$(cat "$d/heartbeat" 2>/dev/null || echo 0)"

        if ! [[ "$hb" =~ ^[0-9]+$ ]]; then
            hb=0
        fi

        if (( hb == 0 )); then
            age=-1
            health_state="NEVER_SEEN"
        else
            age=$(( now - hb ))

            if (( age <= stale_seconds )); then
                health_state="ONLINE"
            else
                health_state="STALE"
            fi
        fi

        if (( age < 0 )); then
            printf '%-3s %-8s %-30s heartbeat=never\n' \
                "$w" "$health_state" "$status"
        else
            printf '%-3s %-8s %-30s heartbeat_age=%ss\n' \
                "$w" "$health_state" "$status" "$age"
        fi
    done
}


# ============================================================================
# Status / inspection
# ============================================================================

status() {
    local w

    for w in w1 w2 w3; do
        printf '%-3s %s\n' "$w" "$(read_status "$w")"
    done
}

inspect_worker() {
    local w="${1:-}"

    validate_worker "$w"

    local d
    d="$(worker_dir "$w")"

    echo "WORKER=$w"
    echo "STATUS=$(read_status "$w")"

    if [[ -f "$d/heartbeat" ]]; then
        echo "HEARTBEAT=$(cat "$d/heartbeat")"
    else
        echo "HEARTBEAT=0"
    fi

    echo
    echo "--- TASK ---"

    if [[ -f "$d/task" ]]; then
        cat "$d/task"
    else
        echo "NO_TASK"
    fi

    echo
    echo "--- RESULT ---"

    if [[ -f "$d/result" ]]; then
        cat "$d/result"
    else
        echo "NO_RESULT"
    fi
}

doctor() {
    local failed=0
    local w d status

    echo "BUS=$BUS"
    echo "COMMON=$COMMON"
    echo

    for w in w1 w2 w3; do
        d="$(worker_dir "$w")"
        status="$(read_status "$w")"

        printf '%s: ' "$w"

        case "$status" in
            idle)
                if [[ -f "$d/task" || -f "$d/result" ]]; then
                    echo "BROKEN: idle but task/result exists"
                    failed=1
                else
                    echo "OK"
                fi
                ;;

            assigned:*)
                if [[ ! -f "$d/task" ]]; then
                    echo "BROKEN: assigned but task missing"
                    failed=1
                else
                    echo "OK"
                fi
                ;;

            working:*)
                if [[ ! -f "$d/task" ]]; then
                    echo "BROKEN: working but task missing"
                    failed=1
                else
                    echo "OK"
                fi
                ;;

            done:*|failed:*)
                if [[ ! -f "$d/task" || ! -f "$d/result" ]]; then
                    echo "BROKEN: terminal state missing task/result"
                    failed=1
                else
                    echo "OK"
                fi
                ;;

            *)
                echo "BROKEN: unknown status '$status'"
                failed=1
                ;;
        esac
    done

    echo

    health

    if (( failed != 0 )); then
        exit 1
    fi
}


# ============================================================================
# Manual recovery
#
# reset is intentionally explicit and destructive to bus metadata only.
# It NEVER changes git branches or working tree contents.
# ============================================================================

reset_worker() {
    local w="${1:-}"

    validate_worker "$w"

    local d
    d="$(worker_dir "$w")"

    (
        flock -x 9

        rm -f "$d/task" "$d/result"
        atomic_write "$d/status" "idle"
        atomic_write "$d/heartbeat" "$(now_epoch)"

    ) 9>"$LOCK"

    echo "$w bus state reset -> idle"
}


# ============================================================================
# Help
# ============================================================================

usage() {
    cat <<EOF
Courier404 multi-agent coordination bus

Usage:
  $SCRIPT_NAME init

Assignment:
  $SCRIPT_NAME assign <w1|w2|w3> <ISSUE_ID> "<prompt>"
  $SCRIPT_NAME poll <w1|w2|w3>
  $SCRIPT_NAME wait <w1|w2|w3> [timeout_seconds] [poll_interval]
  $SCRIPT_NAME start <w1|w2|w3> <ISSUE_ID>

Completion:
  $SCRIPT_NAME done <w1|w2|w3> <ISSUE_ID> <COMMIT> "<evidence>"
  $SCRIPT_NAME fail <w1|w2|w3> <ISSUE_ID> "<reason>"

Integration lifecycle:
  $SCRIPT_NAME results
  $SCRIPT_NAME wait-result [timeout_seconds] [poll_interval]
  $SCRIPT_NAME ack <w1|w2|w3> [ISSUE_ID]
  $SCRIPT_NAME sync <w1|w2|w3>
  $SCRIPT_NAME recycle <w1|w2|w3> [ISSUE_ID]

Monitoring:
  $SCRIPT_NAME status
  $SCRIPT_NAME heartbeat <w1|w2|w3>
  $SCRIPT_NAME health [stale_seconds]
  $SCRIPT_NAME inspect <w1|w2|w3>
  $SCRIPT_NAME doctor

Recovery:
  $SCRIPT_NAME reset <w1|w2|w3>

Recommended worker idle loop:
  while true; do
      $SCRIPT_NAME heartbeat w1
      output="\$($SCRIPT_NAME poll w1)"
      if [[ "\$output" != "NO_TASK" ]]; then
          printf '%s\n' "\$output"
          break
      fi
      sleep 5
  done

Notes:
  * Beads is the authoritative task graph.
  * The bus does NOT own issue status/dependencies.
  * ISSUE must be only the Beads ID.
  * Workers must not run full UE build/test/cook/package/render gates.
  * Orchestrator integrates and verifies one heavy UE lane at a time.
EOF
}


# ============================================================================
# Main
# ============================================================================

init_bus >/dev/null

command="${1:-help}"

case "$command" in
    init)
        init_bus
        ;;

    assign)
        shift
        assign_task "$@"
        ;;

    poll)
        shift
        poll_task "$@"
        ;;

    wait)
        shift
        wait_task "$@"
        ;;

    start)
        shift
        start_task "$@"
        ;;

    done)
        shift
        done_task "$@"
        ;;

    fail)
        shift
        fail_task "$@"
        ;;

    results)
        show_results
        ;;

    wait-result)
        shift
        wait_result "$@"
        ;;

    ack)
        shift
        ack_worker "$@"
        ;;

    sync)
        shift
        sync_worker "$@"
        ;;

    recycle)
        shift
        recycle_worker "$@"
        ;;

    heartbeat)
        shift
        heartbeat "$@"
        ;;

    health)
        shift
        health "$@"
        ;;

    status)
        status
        ;;

    inspect)
        shift
        inspect_worker "$@"
        ;;

    doctor)
        doctor
        ;;

    reset)
        shift
        reset_worker "$@"
        ;;

    help|-h|--help)
        usage
        ;;

    *)
        usage >&2
        die "unknown command: $command"
        ;;
esac
