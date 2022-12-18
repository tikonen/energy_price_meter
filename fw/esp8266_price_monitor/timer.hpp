#pragma once

struct ElapsedTimer {
    uint32_t lastTs;

    ElapsedTimer() { reset(); }

    void reset() { lastTs = millis(); }

    int elapsed()
    {
        uint32_t now = millis();
        int elapsed = now - lastTs;
        lastTs = now;
        return elapsed;
    }
};

// Generic timer utility
struct Timer {
    uint32_t interval;
    uint32_t cumulative = 0;
    int flipflop = 0;  // toggles state on every timer expiry
    const bool autoReset;

    Timer(bool autoRes, uint32_t t)
        : interval(t)
        , autoReset(autoRes)
    {
        reset();
    }

    void reset()
    {
        cumulative = 0;
        flipflop = 0;
    }

    bool update(uint32_t elapseds)
    {
        cumulative += elapseds;
        if (cumulative >= interval) {
            if (autoReset) {
                cumulative = 0;
                flipflop = 1 - flipflop;
            }
            return true;
        }
        return false;
    }
};

struct Timer2 {
    Timer2(bool autoRes, uint32_t t)
        : timer(autoRes, t)
    {
        reset();
    }

    void reset()
    {
        timer.reset();
        lastTs = millis();
    }

    bool update()
    {
        uint32_t ts = millis();
        uint32_t elapsed = ts - lastTs;
        lastTs = ts;
        return timer.update(elapsed);
    }

    int flipflop() const { return timer.flipflop; }

    Timer timer;
    uint32_t lastTs = 0;
};
