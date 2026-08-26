#!/usr/bin/env bash
set -euo pipefail

COMMON="$(git rev-parse --path-format=absolute --git-common-dir)"
BUS="$COMMON/courier-agent-bus"
LOCK="$BUS/.lock"

mkdir -p "$BUS"/{w1,w2,w3}
touch "$LOCK"

worker_dir() {
    case "${1:-}" in
        w1|w2|w3) printf '%s/%s\n' "$BUS" "$1" ;;
        *) echo "worker must be w1, w2, or w3" >&2; exit 2 ;;
    esac
}

init() {
    for w in w1 w2 w3; do
        mkdir -p "$BUS/$w"
        [[ -f "$BUS/$w/status" ]] || printf 'idle\n' > "$BUS/$w/status"
    done
    echo "Agent bus: $BUS"
}

assign() {
    local w="$1" issue="$2"
    shift 2
    local prompt="$*"
    local d
    d="$(worker_dir "$w")"

    flock "$LOCK" bash -c '
        d="$1"; issue="$2"; prompt="$3"
        status="$(cat "$d/status" 2>/dev/null || echo idle)"
        if [[ "$status" != "idle" ]]; then
            echo "worker busy: $status" >&2
            exit 3
        fi
        rm -f "$d/result"
        {
            printf "ISSUE=%s\n" "$issue"
            printf "PROMPT=%s\n" "$prompt"
        } > "$d/task.tmp"
        mv "$d/task.tmp" "$d/task"
        printf "assigned:%s\n" "$issue" > "$d/status"
    ' _ "$d" "$issue" "$prompt"

    echo "$w <- $issue"
}

wait_task() {
    local w="$1"
    local d
    d="$(worker_dir "$w")"

    while :; do
        if [[ -f "$d/task" ]]; then
            cat "$d/task"
            return 0
        fi
        sleep 2
    done
}

start() {
    local w="$1" issue="$2"
    local d
    d="$(worker_dir "$w")"
    printf 'working:%s\n' "$issue" > "$d/status"
}

done_task() {
    local w="$1" issue="$2" commit="$3"
    shift 3
    local evidence="$*"
    local d
    d="$(worker_dir "$w")"

    flock "$LOCK" bash -c '
        d="$1"; issue="$2"; commit="$3"; evidence="$4"
        {
            printf "WORKER=%s\n" "$(basename "$d")"
            printf "ISSUE=%s\n" "$issue"
            printf "COMMIT=%s\n" "$commit"
            printf "EVIDENCE=%s\n" "$evidence"
        } > "$d/result.tmp"
        mv "$d/result.tmp" "$d/result"
        rm -f "$d/task"
        printf "done:%s\n" "$issue" > "$d/status"
    ' _ "$d" "$issue" "$commit" "$evidence"
}

ack() {
    local w="$1"
    local d
    d="$(worker_dir "$w")"
    rm -f "$d/result"
    printf 'idle\n' > "$d/status"
}

fail() {
    local w="$1" issue="$2"
    shift 2
    local reason="$*"
    local d
    d="$(worker_dir "$w")"

    {
        printf "WORKER=%s\n" "$w"
        printf "ISSUE=%s\n" "$issue"
        printf "STATUS=failed\n"
        printf "REASON=%s\n" "$reason"
    } > "$d/result"
    rm -f "$d/task"
    printf 'failed:%s\n' "$issue" > "$d/status"
}


worker_path() {
    local w="$1"
    local branch="refs/heads/agent/$w"
    git worktree list --porcelain | awk -v branch="$branch" '
        $1 == "worktree" { path=$2 }
        $1 == "branch" && $2 == branch { print path; exit }
    '
}

sync_worker() {
    local w="$1"
    local d path status branch
    d="$(worker_dir "$w")"
    status="$(cat "$d/status" 2>/dev/null || echo idle)"
    if [[ "$status" != "idle" && "$status" != done:* ]]; then
        echo "refusing sync: $w status is $status" >&2
        exit 4
    fi
    path="$(worker_path "$w")"
    if [[ -z "$path" ]]; then
        echo "worker worktree not found for agent/$w" >&2
        exit 5
    fi
    if [[ -n "$(git -C "$path" status --porcelain)" ]]; then
        echo "refusing sync: $path is dirty" >&2
        exit 6
    fi
    branch="$(git -C "$path" branch --show-current)"
    if [[ "$branch" != "agent/$w" ]]; then
        echo "refusing sync: expected agent/$w, found $branch" >&2
        exit 7
    fi
    git -C "$path" merge --ff-only main
    echo "$w synced to main"
}

status() {
    for w in w1 w2 w3; do
        printf '%-3s %s\n' "$w" "$(cat "$BUS/$w/status" 2>/dev/null || echo idle)"
    done
}

results() {
    for w in w1 w2 w3; do
        if [[ -f "$BUS/$w/result" ]]; then
            echo "=== $w ==="
            cat "$BUS/$w/result"
        fi
    done
}

wait_result() {
    while :; do
        for w in w1 w2 w3; do
            if [[ -f "$BUS/$w/result" ]]; then
                echo "=== $w ==="
                cat "$BUS/$w/result"
                return 0
            fi
        done
        sleep 2
    done
}

case "${1:-}" in
    init) init ;;
    assign) shift; assign "$@" ;;
    wait) shift; wait_task "$@" ;;
    start) shift; start "$@" ;;
    done) shift; done_task "$@" ;;
    ack) shift; ack "$@" ;;
    fail) shift; fail "$@" ;;
    status) status ;;
    sync) shift; sync_worker "$@" ;;
    results) results ;;
    wait-result) wait_result ;;
    *)
        echo "usage:"
        echo "  $0 init"
        echo "  $0 status"
        echo "  $0 sync w1"
        echo "  $0 assign w1 ISSUE PROMPT"
        echo "  $0 wait w1"
        echo "  $0 start w1 ISSUE"
        echo "  $0 done w1 ISSUE COMMIT EVIDENCE"
        echo "  $0 fail w1 ISSUE REASON"
        echo "  $0 results"
        echo "  $0 wait-result"
        echo "  $0 ack w1"
        exit 2
        ;;
esac
