#pragma once

typedef struct _AnimationDesc {
	int start;
	int end;
	float currentFrame;
	bool looped = false;

	int totalFrames() {
		return this->start - this->end;
	}

	int frameOffset() {
		return start + (int)currentFrame;
	}

	void iterate(float amount) {
		currentFrame += amount;
		if (currentFrame + start >= end) {
			currentFrame = 0;
			looped = true;
		}
	}

	bool checkLooped() {
		if (looped) {
			looped = false;
			return true;
		}
		return false;
	}
} AnimationDesc;