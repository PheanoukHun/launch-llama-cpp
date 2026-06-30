import curses
from curses import wrapper

def main(stdscr):
    # Initialize colors
    curses.start_color()
    curses.init_pair(1, curses.COLOR_RED, curses.COLOR_WHITE)
    curses.init_pair(2, curses.COLOR_GREEN, curses.COLOR_BLACK)
    curses.init_pair(3, curses.COLOR_YELLOW, curses.COLOR_BLACK)

    # Get screen dimensions
    height, width = stdscr.getmaxyx()
    
    # Enable keypad mode for arrow keys
    stdscr.keypad(True)

    # Create a main window for the menu
    # newwin(height, width, begin_y, begin_x)
    main_win = curses.newwin(height - 5, width, 0, 0)
    main_win.attron(curses.A_REVERSE)
    main_win.addstr(0, 0, " TASK MANAGER ", curses.color_pair(1))
    main_win.refresh()

    # Create a status bar at the bottom
    status_win = curses.newwin(2, width, height - 2, 0)
    
    status_msg = " Use Arrow Keys to navigate, Enter to select, 'q' to quit."
    tasks = [
        "Write documentation",
        "Implement TUI feature",
        "Run unit tests",
        "Deploy to production"
    ]
    
    current_selection = 0
    completed_tasks = []

    while True:
        main_win.clear()
        main_win.addstr(1, 2, "Tasks:", curses.A_BOLD)
        
        for i, task in enumerate(tasks):
            if i == current_selection:
                main_win.addstr(i + 2, 4, f"> {task}", curses.color_pair(2))
            else:
                main_win.addstr(i + 2, 4, f"  {task}")
        
        main_win.refresh()
        status_win.addstr(0, 0, status_msg)
        status_win.refresh()

        key = stdscr.getkey()

        if key == curses.KEY_UP:
            current_selection = (current_selection - 1) % len(tasks)
        elif key == curses.KEY_DOWN:
            current_selection = (current_selection + 1) % len(tasks)
        elif key == ord('q') or key == curses.KEY_ESC:
            break
        elif key == curses.KEY_ENTER:
            task_text = tasks[current_selection]
            if task_text not in completed_tasks:
                completed_tasks.append(task_text)
                status_msg = f" COMPLETED: {task_text} "
            else:
                status_msg = f" ALREADY DONE: {task_text} "
            curses.napms(200)

if __name__ == "__main__":
    wrapper(main)
