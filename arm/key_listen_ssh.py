"""
keyboard 库无法获取SSH远程连接的按键，如果要用这个库则必须在树莓派上接键盘（蓝牙键盘也可以）使用
"""

# import curses

# # get the curses screen window
# screen = curses.initscr()

# # turn off input echoing
# curses.noecho()

# # respond to keys immediately (don't wait for enter)
# curses.cbreak()

# # map arrow keys to special values
# screen.keypad(True)

# try:
#     while True:
#         char = screen.getch()
#         if char == ord('q'):
#             break
#         elif char == curses.KEY_RIGHT:
#             # print doesn't work with curses, use addstr instead
#             screen.addstr(0, 0, 'right')
#         elif char == curses.KEY_LEFT:
#             screen.addstr(0, 0, 'left ')
#         elif char == curses.KEY_UP:
#             screen.addstr(0, 0, 'up   ')
#         elif char == curses.KEY_DOWN:
#             screen.addstr(0, 0, 'down ')
# finally:
#     # shut down cleanly
#     curses.nocbreak()
#     screen.keypad(0)
#     curses.echo()
#     curses.endwin()


from sshkeyboard import listen_keyboard
def press(key):
    if key == "up":
        print("up pressed")
    elif key == "down":
        print("down pressed")
    elif key == "left":
        print("left pressed")
    elif key == "right":
        print("right pressed")
listen_keyboard(on_press=press)