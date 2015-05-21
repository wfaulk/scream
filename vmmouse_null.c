/*
 * Large portions taken from shared/vmmouse_client.c from
 * http://www.x.org/releases/individual/driver/xf86-input-vmmouse-13.0.99.tar.gz
 *
 * Original copyright notice below
 */

/*
 * Copyright 2002-2006 by VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 */

#include <stdlib.h>
#include <stdio.h>

#include "vmmouse_defs.h"
#include "vmmouse_proto.h"

/*
 *----------------------------------------------------------------------------
 *
 * VMMouseClientVMCheck --
 *
 *      Checks if we're running in a VM by sending the GETVERSION command.
 *
 * Returns:
 *      0 if we're running natively/the version command failed,
 *      1 if we're in a VM.
 *
 *----------------------------------------------------------------------------
 */

static int VMMouseClientVMCheck(void);
static int
VMMouseClientVMCheck(void)
{
   VMMouseProtoCmd vmpc;

   vmpc.in.vEbx = ~(uint32_t)VMMOUSE_PROTO_MAGIC;
   vmpc.in.command = VMMOUSE_PROTO_CMD_GETVERSION;
   VMMouseProto_SendCmd(&vmpc);

   /*
    * ebx should contain VMMOUSE_PROTO_MAGIC
    * eax should contain version
    */
   if (vmpc.out.vEbx != VMMOUSE_PROTO_MAGIC || vmpc.out.vEax == 0xffffffff) {
      return 0;
   }

   return 1;
}


/*
 *----------------------------------------------------------------------
 *
 * VMMouseClient_Enable --
 *
 *	Public Enable entry point. The driver calls this once it feels
 *	ready to deal with VMMouse stuff. For now, we just try to enable
 *	and return the result, but conceivably we could do more.
 *
 * Results:
 *	TRUE if the enable succeeds, FALSE otherwise.
 *
 * Side effects:
 *	Causes host-side state change.
 *
 *----------------------------------------------------------------------
 */

int VMMouseClient_Enable(void);
int
VMMouseClient_Enable(void)
{

   uint32_t status;
   uint32_t data;
   VMMouseProtoCmd vmpc;

   /*
    * First, make sure we're in a VM; i.e. in dualboot configurations we might
    * find ourselves running on real hardware.
    */

   if (!VMMouseClientVMCheck()) {
      return 0;
   }

   /*
    * We probe for the VMMouse backend by sending the ENABLE
    * command to the mouse. We should get back the VERSION_ID on
    * the data port.
    */
   vmpc.in.vEbx = VMMOUSE_CMD_READ_ID;
   vmpc.in.command = VMMOUSE_PROTO_CMD_ABSPOINTER_COMMAND;
   VMMouseProto_SendCmd(&vmpc);

   /*
    * Check whether the VMMOUSE_VERSION_ID is available to read
    */
   vmpc.in.vEbx = 0;
   vmpc.in.command = VMMOUSE_PROTO_CMD_ABSPOINTER_STATUS;
   VMMouseProto_SendCmd(&vmpc);
   status = vmpc.out.vEax;
   if ((status & 0x0000ffff) == 0) {
      printf("VMMouseClient_Enable: no data on port.");
      return 0;
   }

   /*
    * Get the VMMOUSE_VERSION_ID then
    */
   /* Get just one item */
   vmpc.in.vEbx = 1;
   vmpc.in.command = VMMOUSE_PROTO_CMD_ABSPOINTER_DATA;
   VMMouseProto_SendCmd(&vmpc);
   data = vmpc.out.vEax;
   if (data!= VMMOUSE_VERSION_ID) {
      printf("VMMouseClient_Enable: data was not VERSION_ID");
      return 0;
   }

   /*
    * Restrict access to the VMMouse backdoor handler.
    */
   vmpc.in.vEbx = VMMOUSE_RESTRICT_IOPL;
   vmpc.in.command = VMMOUSE_PROTO_CMD_ABSPOINTER_RESTRICT;
   VMMouseProto_SendCmd(&vmpc);

   /*
    * To quote Jeremy, "Go Go Go!"
    */

   return 1;
}


/*
 *----------------------------------------------------------------------------
 *
 * VMMouseClient_RequestRelative --
 *
 *      Request that the host switch to posting relative packets. It's just
 *      advisory, so we make no guarantees about if/when the switch will
 *      happen.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Host may start posting relative packets in the near future.
 *
 *----------------------------------------------------------------------------
 */

void VMMouseClient_RequestRelative(void);
void
VMMouseClient_RequestRelative(void)
{
   VMMouseProtoCmd vmpc;

   vmpc.in.vEbx = VMMOUSE_CMD_REQUEST_RELATIVE;
   vmpc.in.command = VMMOUSE_PROTO_CMD_ABSPOINTER_COMMAND;
   VMMouseProto_SendCmd(&vmpc);
}


/*
 *----------------------------------------------------------------------------
 *
 * VMMouseClient_RequestAbsolute --
 *
 *      Request that the host switch to posting absolute packets. It's just
 *      advisory, so we make no guarantees about if/when the switch will
 *      happen.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Host may start posting absolute packets in the near future.
 *
 *----------------------------------------------------------------------------
 */

void VMMouseClient_RequestAbsolute(void);
void
VMMouseClient_RequestAbsolute(void)
{
   VMMouseProtoCmd vmpc;

   vmpc.in.vEbx = VMMOUSE_CMD_REQUEST_ABSOLUTE;
   vmpc.in.command = VMMOUSE_PROTO_CMD_ABSPOINTER_COMMAND;
   VMMouseProto_SendCmd(&vmpc);
}


static char *prog;

void usage(void) __attribute__ ((noreturn));
void
usage(void)
{
   printf("Usage: %s [-e|-d]\n", prog);
   exit(-1);
}


int
main(int argc, char *argv[])
{
   int enable = -1;
   char ch;

   prog = argv[0];

   while ((ch = (char)getopt(argc, argv, "de")) != -1) {
      switch (ch) {
         case 'd':
            if ( enable != -1 ) {
               usage();
            }
            enable = 0;
            break;
         case 'e':
            if ( enable != -1 ) {
               usage();
            }
            enable = 1;
            break;
         case '?':
         default:
            usage();
      }
   }
   argc -= optind;
   argv += optind;
   if ( argc > 0 ) {
      usage();
   }
   if ( enable == -1 ) {
      enable = 1;
   }


   if ( ! VMMouseClient_Enable() ) {
      printf("VM Mouse not available. Exiting\n");
      return 1;
   }

   if ( enable ) {
      VMMouseClient_RequestAbsolute();
   } else {
      VMMouseClient_RequestRelative();
   }

   return 0;
}
