#ifndef AROS_AROS_H
#define AROS_AROS_H

#if defined(__MORPHOS__)

#define AROS_UFHA(a, b, c) a b = ( a ) REG_ ## c
#define AROS_USERFUNC_INIT
#define AROS_USERFUNC_EXIT }

#define AROS_UFH3(a, b, c, d, e) \
a b ##PPC(void); \
static struct EmulLibEntry b = { TRAP_LIB, 0, (APTR) & b ##PPC }; \
a b ##PPC(void) { c ; d ; e ;

#define BOOPSI_DISPATCHER(a, b, c, d, e) \
a b ##PPC(void); \
static struct EmulLibEntry b = { TRAP_LIB, 0, (APTR) & b ##PPC }; \
a b ##PPC(void) { struct IClass * c = (APTR)REG_A0; APTR d = (APTR)REG_A2; Msg e = (APTR)REG_A1;

#define BOOPSI_DISPATCHER_END }

ULONG XGET(APTR, ULONG);
APTR ImageButton(CONST_STRPTR text, CONST_STRPTR image);

#endif	

#endif /* AROS_AROS_H */
