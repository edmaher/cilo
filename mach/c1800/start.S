/* CILO entry point for the Cisco 1800 Series Routers
 * (C) 2024 Ed Maher
 * Licensed under the GNU General Public License v2.0 or later. See
 * COPYING in the root of the source distribution for more details.
 */

/*
If we use 0xff704600 there is no crash, but it fails
If we use 0xff704500 there is console output, even thought the sh platform says that's for AUX port
*/

#include <asm/ppc_asm.h>

.extern start_bootloader

.rodata
startmessage:
    .ascii  "CILO Starting...\r\nxxxxx"
startmessage_b:
    .ascii  "Calling C\r\n"

.text
.global _start

_start:
    stwu    r1,-16(r1)
    mflr    r0
    stw     r0, 8(r1)

    lis     r26, 0x0  # I wonder if physical address FF70 is remapped to 0000 ?
    lis     r26, 0xff70  ### UART upper BASE for N162552
    addi    r26,r26, 0x4500  ### UART lower BASE address

    lis     r3,startmessage@ha
    addi    r3,r3,startmessage@l
    li      r4, 0x12  ##length
    bl      send_string

    lis     r3,startmessage_b@ha
    addi    r3,r3,startmessage_b@l
    li      r4, 0x0b  ##length
    bl      send_string

    /* jump to the C code */
    bl start_bootloader

# ret
end:
    lwz     r0, 8(r1)
    mtlr    r0
    addi     r1, r1, 16
    blr


#
# SEND STRING
# r3: string
# r4: length
#
send_string:
    stwu    r1,-16(r1)
    mflr    r0,
    stw     r0, 8(r1)

    # init var
    mr      r6,r3 # string
    mr      r7,r4 # length
    li      r8,0  # counter

    # send loop
    send_string_next_char:
    lbzx    r3, r6, r8
    bl      send_char
    addi    r8,r8,1
    cmp     0,0,r8,r7
    blt     send_string_next_char

    # ret
    send_string_end:
    lwz     r0, 8(r1)
    mtlr    r0
    addi    r1, r1, 16
    blr

#
# SEND CHAR
# r3: character
#
send_char:
    stwu    r1,-16(r1)
    mflr    r0
    stw     r0, 8(r1)

    # wait for transmit buffer ready
    loop_transmit_buffer_ready:
    lbz     r10, 5(r26)
    andi.   r10, r10, 0x20
    beq     0,loop_transmit_buffer_ready

    # put  char in transmission buffer
    transmit_char:
    stb     r3, 0(r26)

    # ret
    ret:
    lwz     r0, 8(r1)
    mtlr    r0
    addi    r1, r1, 16
    blr

.end _start
.size _start, .-_start

