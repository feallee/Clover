#include <stddef.h>
#include "Mealy.h"

Mealy_ErrorType Mealy_Start(Mealy_MachineType *machine,
							const Mealy_StateType *states,
							uint32_t stateLength,
							uint32_t transitionLength)
{
	if ((machine == NULL) || (states == NULL))
	{
		return MEALY_ERROR_NULL;
	}
	if ((stateLength < 2) || (transitionLength < 1))
	{
		return MEALY_ERROR_RANGE;
	}
	for (uint32_t i = 0; i < stateLength; i++)
	{
		if (states[i].Transitions == NULL)
		{
			return MEALY_ERROR_NULL;
		}
	}
	machine->Current = 0;
	machine->States = states;
	machine->StateLength = stateLength;
	machine->TransitionLength = transitionLength;
	return MEALY_ERROR_NONE;
}

Mealy_ErrorType Mealy_Stop(Mealy_MachineType *machine)
{
	if (machine == NULL)
	{
		return MEALY_ERROR_NULL;
	}
	machine->Current = machine->StateLength;
	return MEALY_ERROR_NONE;
}

Mealy_ErrorType Mealy_Raise(Mealy_MachineType *machine,
							uint32_t event,
							void *parameter)
{
	const Mealy_TransitionType *t;

	if (machine == NULL)
	{
		return MEALY_ERROR_NULL;
	}
	if (machine->Current >= machine->StateLength)
	{
		return MEALY_ERROR_IGNORED_FINAL;
	}
	if (event >= machine->TransitionLength)
	{
		return MEALY_ERROR_RANGE;
	}
	if (machine->States == NULL)
	{
		return MEALY_ERROR_NULL;
	}

	t = machine->States[machine->Current].Transitions;
	if (t == NULL)
	{
		return MEALY_ERROR_NULL;
	}
	t = &t[event];
	if (t->Next == 0U)
	{
		return MEALY_ERROR_IGNORED_INITIAL;
	}

	if (t->Handler != NULL)
	{
		t->Handler(machine->Current, t->Next, event, parameter);
	}
	machine->Current = t->Next;
	return MEALY_ERROR_NONE;
}
