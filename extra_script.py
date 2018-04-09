# Fuses for ATTiny13A @ 128Khz
Import('env')
env.Replace(FUSESCMD="avrdude $UPLOADERFLAGS -B 250 -e -Uhfuse:w:0xFF:m -Ulfuse:w:0x7B:m")
