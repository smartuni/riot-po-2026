ifdef CI
  # on the CI machine there's an external UART instead of the USB interface
  PORT=/dev/ttyUSB0
  USEMODULE += stdio_uart

  # override the reset command and the programmer reset (see RIOT's reset Make target)
  RESET = "true"
  _PROGRAMMER_RESET = stty -F $(PORT) raw ispeed 1200 ospeed 1200 cs8 -cstopb ignpar eol 255 eof 255
  PREFLASHER = board_helper --port $(PORT)
  PREFFLAGS =
  PREFLASH_DELAY=10
  TERMFLAGS += --set-dtr=0
endif
