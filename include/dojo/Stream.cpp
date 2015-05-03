#include "stdafx.h"

#include "Stream.h"

using namespace Dojo;

int Stream::read(byte* buf, int number) {
	FAIL("This Stream is not read-enabled");
}

void Stream::write(byte* buf, int size) {
	FAIL("This stream is not write-enabled");
}

Stream* Stream::copy() {
	FAIL("This Stream type does not implement copy");
}
