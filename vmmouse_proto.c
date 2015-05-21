#include "vmmouse_proto.h"


/*
 *----------------------------------------------------------------------------
 *
 * VMMouseProtoInOut --
 *
 *      Send a low-bandwidth basic request (16 bytes) to vmware, and return its
 *      reply (24 bytes).
 *
 * Results:
 *      Host-side response returned in cmd IN/OUT parameter.
 *
 * Side effects:
 *      Pokes the communication port.
 *
 *----------------------------------------------------------------------------
 */

static void
VMMouseProtoInOut(VMMouseProtoCmd *cmd) // IN/OUT
{
#ifdef __x86_64__
   uint64_t dummy;

   __asm__ __volatile__(
        "pushq %%rax"           "\n\t"
        "movq 40(%%rax), %%rdi" "\n\t"
        "movq 32(%%rax), %%rsi" "\n\t"
        "movq 24(%%rax), %%rdx" "\n\t"
        "movq 16(%%rax), %%rcx" "\n\t"
        "movq  8(%%rax), %%rbx" "\n\t"
        "movq   (%%rax), %%rax" "\n\t"
        "inl %%dx, %%eax"       "\n\t"  /* NB: There is no inq instruction */
        "xchgq %%rax, (%%rsp)"  "\n\t"
        "movq %%rdi, 40(%%rax)" "\n\t"
        "movq %%rsi, 32(%%rax)" "\n\t"
        "movq %%rdx, 24(%%rax)" "\n\t"
        "movq %%rcx, 16(%%rax)" "\n\t"
        "movq %%rbx,  8(%%rax)" "\n\t"
        "popq          (%%rax)"
      : "=a" (dummy)
      : "0" (cmd)
      /*
       * vmware can modify the whole VM state without the compiler knowing
       * it. So far it does not modify EFLAGS. --hpreg
       */
      : "rbx", "rcx", "rdx", "rsi", "rdi", "memory"
   );
#else
#ifdef __i386__
   uint32_t dummy;

   __asm__ __volatile__(
        "pushl %%ebx"           "\n\t"
        "pushl %%eax"           "\n\t"
        "movl 20(%%eax), %%edi" "\n\t"
        "movl 16(%%eax), %%esi" "\n\t"
        "movl 12(%%eax), %%edx" "\n\t"
        "movl  8(%%eax), %%ecx" "\n\t"
        "movl  4(%%eax), %%ebx" "\n\t"
        "movl   (%%eax), %%eax" "\n\t"
        "inl %%dx, %%eax"       "\n\t"
        "xchgl %%eax, (%%esp)"  "\n\t"
        "movl %%edi, 20(%%eax)" "\n\t"
        "movl %%esi, 16(%%eax)" "\n\t"
        "movl %%edx, 12(%%eax)" "\n\t"
        "movl %%ecx,  8(%%eax)" "\n\t"
        "movl %%ebx,  4(%%eax)" "\n\t"
        "popl          (%%eax)" "\n\t"
        "popl           %%ebx"
      : "=a" (dummy)
      : "0" (cmd)
      /*
       * vmware can modify the whole VM state without the compiler knowing
       * it. So far it does not modify EFLAGS. --hpreg
       */
      : "ecx", "edx", "esi", "edi", "memory"
   );
#else
#error "VMMouse is only supported on x86 and x86-64."
#endif
#endif
}


/*
 *-----------------------------------------------------------------------------
 *
 * VMMouseProto_SendCmd --
 *
 *      Send a request (16 bytes) to vmware, and synchronously return its
 *      reply (24 bytes).
 *
 * Result:
 *      None
 *
 * Side-effects:
 *      None
 *
 *-----------------------------------------------------------------------------
 */

void
VMMouseProto_SendCmd(VMMouseProtoCmd *cmd) // IN/OUT
{
   cmd->in.magic = VMMOUSE_PROTO_MAGIC;
   cmd->in.port = VMMOUSE_PROTO_PORT;

   VMMouseProtoInOut(cmd);
}
