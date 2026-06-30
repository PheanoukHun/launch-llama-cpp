import curses


def selection(stdscr, title, options, default_idx=0):
    """Arrow-key navigable selection menu. Returns selected option or None."""
    current = default_idx if default_idx is not None else 0

    while True:
        stdscr.clear()
        height, width = stdscr.getmaxyx()

        stdscr.addstr(0, 2, title, curses.A_BOLD)

        sep = "─" * min(width - 2, 60)
        stdscr.addstr(1, 0, sep)

        for i, opt in enumerate(options):
            y = i + 3
            if y >= height - 2:
                break
            display = str(opt)
            if len(display) > width - 8:
                display = display[:width - 11] + "..."
            if i == current:
                stdscr.addstr(y, 2, "▸")
                stdscr.addstr(y, 4, display, curses.A_REVERSE)
            else:
                stdscr.addstr(y, 4, display)

        stdscr.addstr(height - 1, 0, "↑↓ navigate | Enter select | q quit")
        stdscr.refresh()

        key = stdscr.getch()

        if key == curses.KEY_UP:
            current = (current - 1) % len(options)
        elif key == curses.KEY_DOWN:
            current = (current + 1) % len(options)
        elif key in (ord('\n'), ord('\r'), curses.KEY_ENTER):
            return options[current]
        elif key in (ord('q'), 27, 3):
            return None


def integer(stdscr, title, min_val=None, max_val=None, default=None):
    """Number input within curses. Returns integer, default, or None."""
    stdscr.clear()
    height, width = stdscr.getmaxyx()

    stdscr.addstr(0, 2, title, curses.A_BOLD)
    stdscr.addstr(1, 0, "─" * min(width - 2, 60))

    y = 3
    if min_val is not None and max_val is not None:
        stdscr.addstr(y, 2, f"Range: {min_val} - {max_val}")
        y += 1
    if default is not None:
        stdscr.addstr(y, 2, f"Default: {default}")
        y += 1

    y += 1
    prompt = "Enter number: "
    stdscr.addstr(y, 2, prompt)
    stdscr.refresh()

    try:
        curses.curs_set(1)
    except:
        pass

    buf = []
    pos = 0
    max_input = 15
    input_x = 2 + len(prompt)

    while True:
        key = stdscr.getch()

        if key in (ord('\n'), ord('\r'), curses.KEY_ENTER):
            break
        elif key in (curses.KEY_UP, curses.KEY_DOWN, curses.KEY_LEFT, curses.KEY_RIGHT):
            continue
        elif key in (ord('q'), 27, 3):
            try:
                curses.curs_set(0)
            except:
                pass
            return None
        elif key in (curses.KEY_BACKSPACE, 127, ord('\b')):
            if buf:
                buf.pop()
                pos = len(buf)
                stdscr.addstr(y, input_x, " " * max_input)
                stdscr.addstr(y, input_x, "".join(buf))
                stdscr.move(y, input_x + pos)
                stdscr.refresh()
        elif ord('0') <= key <= ord('9'):
            if len(buf) < max_input:
                buf.append(chr(key))
                pos = len(buf)
                stdscr.addstr(y, input_x + pos - 1, chr(key))
                stdscr.refresh()
        else:
            pass

    try:
        curses.curs_set(0)
    except:
        pass

    raw = "".join(buf).strip()
    if not raw:
        return default
    try:
        value = int(raw)
        if min_val is not None and value < min_val:
            return default
        if max_val is not None and value > max_val:
            return default
        return value
    except ValueError:
        return default


def confirm(stdscr, title, default=True):
    """Yes/no confirmation within curses. Returns bool or None."""
    stdscr.clear()
    height, width = stdscr.getmaxyx()

    stdscr.addstr(0, 2, title, curses.A_BOLD)
    stdscr.addstr(1, 0, "─" * min(width - 2, 60))

    hint = "(Y/n): " if default else "(y/N): "
    stdscr.addstr(3, 2, hint)
    stdscr.refresh()

    try:
        curses.curs_set(1)
    except:
        pass

    buf = []
    while True:
        key = stdscr.getch()
        if key in (ord('\n'), ord('\r'), curses.KEY_ENTER):
            break
        elif key in (curses.KEY_UP, curses.KEY_DOWN, curses.KEY_LEFT, curses.KEY_RIGHT):
            continue
        elif key in (ord('q'), 27, 3):
            try:
                curses.curs_set(0)
            except:
                pass
            return None
        elif key in (curses.KEY_BACKSPACE, 127, ord('\b')):
            if buf:
                buf.pop()
                stdscr.addstr(3, 2 + len(hint), " ")
                stdscr.move(3, 2 + len(hint))
                stdscr.refresh()
        elif chr(key).lower() in ('y', 'n'):
            buf.append(chr(key).lower())
            stdscr.addstr(3, 2 + len(hint), chr(key).lower())
            stdscr.refresh()
        else:
            pass

    try:
        curses.curs_set(0)
    except:
        pass

    raw = "".join(buf).strip().lower()
    if not raw:
        return default
    if raw == 'y':
        return True
    if raw == 'n':
        return False
    return default
