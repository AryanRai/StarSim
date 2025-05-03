#pragma once

namespace mathcore {

    inline float add(float a, float b) {
        return a + b;
    }

    inline float subtract(float a, float b) {
        return a - b;
    }

    inline float multiply(float a, float b) {
        return a * b;
    }

    inline float divide(float a, float b) {
        return (b != 0.0f) ? a / b : 0.0f;  // Prevent divide-by-zero
    }

}
