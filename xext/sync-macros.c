#include <X11/Xlib.h>
#include <X11/extensions/sync.h>

void
XSyncIntToValue(XSyncValue *pv, int i)
{
	_XSyncIntToValue(pv,i);
}

void
XSyncIntsToValue(XSyncValue *pv, unsigned int l, int h)
{
	_XSyncIntsToValue(pv, l, h);
}

Bool
XSyncValueGreaterThan(XSyncValue a, XSyncValue b)
{
	return _XSyncValueGreaterThan(a, b);
}

Bool
XSyncValueLessThan(XSyncValue a, XSyncValue b)
{
	return _XSyncValueLessThan(a, b);
}

Bool
XSyncValueGreaterOrEqual(XSyncValue a, XSyncValue b)
{
	return _XSyncValueGreaterOrEqual(a, b);
}

Bool
XSyncValueLessOrEqual(XSyncValue a, XSyncValue b)
{
	return _XSyncValueLessOrEqual(a, b);
}

Bool
XSyncValueEqual(XSyncValue a, XSyncValue b)
{
	return _XSyncValueEqual(a, b);
}

Bool
XSyncValueIsNegative(XSyncValue v)
{
	return _XSyncValueIsNegative(v);
}

Bool
XSyncValueIsZero(XSyncValue a)
{
	return _XSyncValueIsZero(a);
}

Bool
XSyncValueIsPositive(XSyncValue v)
{
	return _XSyncValueIsPositive(v);
}

unsigned int
XSyncValueLow32(XSyncValue v)
{
	return _XSyncValueLow32(v);
}

int
XSyncValueHigh32(XSyncValue v)
{
	return _XSyncValueHigh32(v);
}

void
XSyncValueAdd(XSyncValue *presult, XSyncValue a, XSyncValue b, Bool *poverflow)
{
	_XSyncValueAdd(presult, a, b, poverflow);
}

void
XSyncValueSubtract(
	XSyncValue *presult,
	XSyncValue a, XSyncValue b,
	Bool *poverflow)
{
	_XSyncValueSubtract(presult, a, b, poverflow);
}

void
XSyncMaxValue(XSyncValue *pv)
{
	_XSyncMaxValue(pv);
}

void
XSyncMinValue(XSyncValue *pv)
{
	_XSyncMinValue(pv);
}
