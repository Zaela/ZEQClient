
#ifndef _ZEQ_EQ_STATE_H
#define _ZEQ_EQ_STATE_H
enum EqState
{
	None,
	Login,
	LoginServer,
	World,
	CharSel,
	Zone,
	MAX_STATE	
};

extern EqState g_EqState;
#endif