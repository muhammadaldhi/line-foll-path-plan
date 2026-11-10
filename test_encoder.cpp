#include <cassert>
#include <cstdint>

#include "encoder.h"

void test_forward_sequence_increments() {
    EncoderController encoder;
    encoder.updateLeft(false, false);
    encoder.updateLeft(false, true);
    encoder.updateLeft(true, true);
    encoder.updateLeft(true, false);
    encoder.updateLeft(false, false);
    assert(encoder.leftCount() == 4);
}

void test_reverse_sequence_decrements() {
    EncoderController encoder;
    encoder.updateRight(false, false);
    encoder.updateRight(true, false);
    encoder.updateRight(true, true);
    encoder.updateRight(false, true);
    encoder.updateRight(false, false);
    assert(encoder.rightCount() == -4);
}

void test_invalid_transition_is_ignored() {
    EncoderController encoder;
    encoder.updateLeft(false, false);
    encoder.updateLeft(true, true);
    assert(encoder.leftCount() == 0);
}

void test_counts_are_signed() {
    EncoderController encoder;
    encoder.updateLeft(false, false);
    encoder.updateLeft(true, false);
    assert(encoder.leftCount() < 0);
}

void test_reset_clears_counts() {
    EncoderController encoder;
    encoder.updateLeft(false, false);
    encoder.updateLeft(false, true);
    encoder.updateRight(false, false);
    encoder.updateRight(true, false);
    encoder.reset();
    assert(encoder.leftCount() == 0);
    assert(encoder.rightCount() == 0);
}

int main() {
    test_forward_sequence_increments();
    test_reverse_sequence_decrements();
    test_invalid_transition_is_ignored();
    test_counts_are_signed();
    test_reset_clears_counts();
}
