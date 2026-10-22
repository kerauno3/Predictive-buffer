// Predictive Buffer - original implementation of a jump input-buffer assist.
//
// Idea: if you press jump while airborne, remember the press for N physics
// steps. If you land inside that window, the jump is fired on the first step
// you are grounded, even if you've already released the button.
//
// Window sizes are measured in physics steps (GD runs 240 steps/sec by
// default, so 10 steps ~ 41ms, 5 ~ 21ms, 2 ~ 8ms).
//
// NOTE: written against the Geode bindings without being compiled/tested here.
// Member names (m_isOnGround, m_touchingRings, ...) and the keyboard dispatcher
// signature can differ slightly between GD/Geode versions - fix any compile
// errors against your SDK's bindings.

#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/ui/Notification.hpp>

using namespace geode::prelude;

namespace {
	// Modes cycle: 0 = 10 steps, 1 = 5 steps, 2 = 2 steps, 3 = off
	constexpr int WINDOWS[] = {10, 5, 2, 0};
	constexpr int MODE_COUNT = 4;

	int g_mode = 3;           // start disabled until the user opts in
	bool g_assistUsed = false; // did the buffer fire during this run?

	int currentWindow() { return WINDOWS[g_mode]; }

	std::string modeLabel() {
		int w = currentWindow();
		if (w == 0) return "Predictive Buffer: OFF";
		return fmt::format("Predictive Buffer: {} steps", w);
	}
}

$on_mod(Loaded) {
	g_mode = Mod::get()->getSavedValue<int>("mode", 3);
	if (g_mode < 0 || g_mode >= MODE_COUNT) g_mode = 3;
}

// ---------------------------------------------------------------- F6 toggle
class $modify(BufferKeyboard, CCKeyboardDispatcher) {
	bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat) {
		if (down && !repeat && key == KEY_F6) {
			g_mode = (g_mode + 1) % MODE_COUNT;
			Mod::get()->setSavedValue("mode", g_mode);
			Notification::create(modeLabel(), NotificationIcon::Info, 1.f)->show();
		}
		return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat);
	}
};

// ------------------------------------------------------------ buffer logic
class $modify(BufferPlayer, PlayerObject) {
	struct Fields {
		int bufferLeft = 0;       // steps remaining in the buffered click
		bool physicallyHeld = false;
		bool releasePending = false; // release after the buffered jump is processed
	};

	// Modes where a click is a one-shot jump/flip that benefits from buffering.
	bool bufferableMode() {
		return !(m_isShip || m_isBird || m_isDart || m_isSwing);
	}

	bool touchingRing() {
		return m_touchingRings && m_touchingRings->count() > 0;
	}

	bool pushButton(PlayerButton btn) {
		if (btn == PlayerButton::Jump) {
			m_fields->physicallyHeld = true;

			int window = currentWindow();
			// Only buffer a click made in the air and not meant for an orb.
			if (window > 0 && bufferableMode() && !m_isOnGround && !touchingRing()) {
				m_fields->bufferLeft = window;
			}
		}
		return PlayerObject::pushButton(btn);
	}

	bool releaseButton(PlayerButton btn) {
		if (btn == PlayerButton::Jump) {
			m_fields->physicallyHeld = false;
		}
		return PlayerObject::releaseButton(btn);
	}

	// An orb consumed the click -> don't also jump on landing.
	void ringJump(RingObject* ring, bool p1) {
		m_fields->bufferLeft = 0;
		PlayerObject::ringJump(ring, p1);
	}

	void resetObject() {
		m_fields->bufferLeft = 0;
		m_fields->releasePending = false;
		m_fields->physicallyHeld = false;
		PlayerObject::resetObject();
	}

	void update(float dt) {
		auto f = m_fields.self();

		if (currentWindow() == 0) {
			f->bufferLeft = 0;
		}

		// Grounded with a pending buffered click: fire the jump now.
		if (f->bufferLeft > 0 && m_isOnGround && bufferableMode()) {
			f->bufferLeft = 0;
			g_assistUsed = true;
			PlayerObject::pushButton(PlayerButton::Jump);
			// If the player already let go, release again after this step so
			// we don't leave the button stuck down.
			f->releasePending = !f->physicallyHeld;
		}

		PlayerObject::update(dt);

		if (f->releasePending) {
			f->releasePending = false;
			PlayerObject::releaseButton(PlayerButton::Jump);
		}

		if (f->bufferLeft > 0) {
			f->bufferLeft--;
		}
	}
};

// ------------------------------------------------------------- safe mode
class $modify(BufferPlayLayer, PlayLayer) {
	void setupHasCompleted() {
		PlayLayer::setupHasCompleted();
		g_assistUsed = false;
	}

	void levelComplete() {
		// Treat assisted completions like test-mode runs so they aren't
		// submitted as legit records.
		if (g_assistUsed && Mod::get()->getSettingValue<bool>("safe-mode")) {
			m_isTestMode = true;
		}
		PlayLayer::levelComplete();
	}
};
