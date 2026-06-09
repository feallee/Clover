#include <stddef.h>
#include "Mealy.h"

Mealy_ReturnType Mealy_Start(Mealy_MachineType* machine,
	const Mealy_StateType* states,
	uint32_t stateLength,
	uint32_t transitionLength)
{
	if ((machine == NULL) || (states == NULL))
	{
		return MEALY_RETURN_NULL;
	}
	if ((stateLength < 2) || (transitionLength < 1))
	{
		return MEALY_RETURN_OVERFLOW;
	}
	for (uint32_t i = 0; i < stateLength; i++)
	{
		if (states[i].Transitions == NULL)
		{
			return MEALY_RETURN_NULL;
		}
	}
	machine->Current = 0; /*默认起始状态固定为 0*/
	machine->States = states;
	machine->StateLength = stateLength;
	machine->TransitionLength = transitionLength;
	return MEALY_RETURN_OK;
}

Mealy_ReturnType Mealy_Stop(Mealy_MachineType* machine)
{
	if (machine == NULL)
	{
		return MEALY_RETURN_NULL;
	}
	machine->Current = machine->StateLength; /*设置为最终状态，最终状态大于等于状态数量*/
	return MEALY_RETURN_OK;
}

Mealy_ReturnType Mealy_Raise(Mealy_MachineType* machine,
	uint32_t event,
	void* parameter)
{
	if (machine == NULL)
	{
		return MEALY_RETURN_NULL;
	}
	if (machine->Current >= machine->StateLength) /*最终状态，啥也不做*/
	{
		return MEALY_RETURN_IGNORED_FINAL;
	}
	if (event >= machine->TransitionLength)
	{
		return MEALY_RETURN_OVERFLOW;
	}
	if (machine->States == NULL)
	{
		return MEALY_RETURN_NULL;
	}

	const Mealy_TransitionType* t = machine->States[machine->Current].Transitions; /*O(1) 查表实现*/
	if (t == NULL)
	{
		return MEALY_RETURN_NULL;
	}
	t = &t[event];
	if (t->Next == 0U) /*不允许再进入起始状态，啥也不做*/
	{
		return MEALY_RETURN_IGNORED_INITIAL;
	}

	if (t->Handler != NULL)
	{
		t->Handler(machine->Current, t->Next, event, parameter);
	}
	machine->Current = t->Next;
	return MEALY_RETURN_OK;
}
