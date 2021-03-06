#pragma once

#include "dojo_common_header.h"

#include "InputDevice.h"

namespace Dojo {
	///A keyboard represents a single key mapping abstracted on the keyboard
	/**\remark multiple Keyboards can exist at the same time, allowing for same-keyboard multiplayer*/
	class Keyboard : public InputDevice {
	public:

		struct FakeAxis {
			Axis axis;
			KeyCode min, max;

			FakeAxis() {}

			FakeAxis(Axis axis, KeyCode min, KeyCode max) :
				axis(axis),
				min(min),
				max(max) {

			}

			bool operator ==(const FakeAxis& other) const {
				return axis == other.axis and min == other.min and max == other.max;
			}
		};

		typedef SmallSet<FakeAxis> FakeAxes;

		//a keyboard has n buttons (KC_JOYPAD_1 comes right after the KB button defs, and 2 fake axes, LX and LY
		Keyboard();

		virtual ~Keyboard() {
		}

		void addFakeAxis(Axis axis, KeyCode min, KeyCode max);

		virtual void clearBindings() override;

		virtual void poll(float dt) override;

		virtual bool hasAxis(Axis a) const override;

	private:

		FakeAxes mFakeAxes;
	private:
	};
}
