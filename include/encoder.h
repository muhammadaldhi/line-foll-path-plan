#ifndef ENCODER_H
#define ENCODER_H

#include <cstdint>

class EncoderController {
public:
    EncoderController();
    void updateLeft(bool a, bool b);
    void updateRight(bool a, bool b);
    int32_t leftCount() const;
    int32_t rightCount() const;
    void reset();

private:
    int32_t leftCount_;
    int32_t rightCount_;
    uint8_t leftState_;
    uint8_t rightState_;
};

#endif
