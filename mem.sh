#!/bin/bash
set -u

descendants() {
    local root=$1
    local queue=("$root")
    local seen=""
    local p c
    while [ ${#queue[@]} -gt 0 ]; do
        p=${queue[0]}
        queue=("${queue[@]:1}")
        case " $seen " in *" $p "*) continue ;; esac
        seen="$seen $p"
        echo "$p"
        [ -r "/proc/$p/task/$p/children" ] || continue
        for c in $(cat "/proc/$p/task/$p/children" 2>/dev/null); do
            queue+=("$c")
        done
    done
}

rss_kb() {
    awk '/^VmRSS:/{print $2}' "/proc/$1/status" 2>/dev/null
}

sum_rss() {
    local total=0 p v
    while read -r p; do
        v=$(rss_kb "$p")
        [ -n "${v:-}" ] && total=$((total + v))
    done < <(descendants "$1")
    echo "$total"
}

"$@" &
pid=$!

max=0
while kill -0 "$pid" 2>/dev/null; do
    s=$(sum_rss "$pid")
    if [ -n "${s:-}" ] && [ "$s" -gt "$max" ]; then
        max=$s
    fi
    sleep 0.03
done

s=$(rss_kb "$pid")
if [ -n "${s:-}" ] && [ "$s" -gt "$max" ]; then
    max=$s
fi

wait "$pid"; rc=$?

awk -v k="$max" 'BEGIN{printf "mem_usage: %.1fMb\n", k/1024.0}' >&2

exit $rc