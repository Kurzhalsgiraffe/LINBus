#include <iostream>

int main() {
    const uint8_t ARRAY_SIZE = 64;
    const uint8_t LIN_SEGMENT_LENGTH = 10;
    const uint8_t LIN_MESSAGE_LENGTH = 5 * LIN_SEGMENT_LENGTH;

    uint8_t buffer[ARRAY_SIZE] = {
        0, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 1, 0, 1, 1, 0, 1, 0, 1,
        1, 0, 0, 1, 0, 0, 1, 1, 0, 1,
        0, 1, 0, 0, 0, 0, 0, 1, 0, 0,
        1, 0, 1, 0, 0, 0, 1, 1, 0, 1,
        1, 0, 0, 0, 1, 1, 0, 0
    };

    uint8_t zeroCount = 0;
    int8_t syncBreakIndex = -1; // Index of last SyncBreak bit
    uint8_t writePointer = 2;   // Index of the last written data to the buffer

    // Find SyncBreak
    for (uint8_t i = 0; i < ARRAY_SIZE * 2; i++) {
        uint8_t idx = i % ARRAY_SIZE;

        if (buffer[idx] == 0) {
            zeroCount++;
        } else {
            zeroCount = 0;
        }

        if (zeroCount >= 13 && buffer[(idx + 1) % ARRAY_SIZE] == 1) {
            syncBreakIndex = (idx + 1) % ARRAY_SIZE;
            std::cout << "SyncBreak found at index: " << (int)syncBreakIndex << std::endl;
            break;
        }
    }

    if (syncBreakIndex != -1) {
        // Ensure that the message is complete in the buffer
        if ((writePointer -1 + ARRAY_SIZE - syncBreakIndex) % ARRAY_SIZE >= LIN_MESSAGE_LENGTH) {
            int messageX[LIN_SEGMENT_LENGTH];
            int messageY[LIN_SEGMENT_LENGTH];
            int messageZ[LIN_SEGMENT_LENGTH];

            // Read X, Y, Z from the buffer
            for (uint8_t j = 0; j < LIN_SEGMENT_LENGTH; ++j) {
                messageX[j] = buffer[(syncBreakIndex + 1 + j) % ARRAY_SIZE];
                messageY[j] = buffer[(syncBreakIndex + 1 + LIN_SEGMENT_LENGTH + j) % ARRAY_SIZE];
                messageZ[j] = buffer[(syncBreakIndex + 1 + 2 * LIN_SEGMENT_LENGTH + j) % ARRAY_SIZE];
            }

            // Output the results
            std::cout << "MessageX: ";
            for (uint8_t i = 0; i < LIN_SEGMENT_LENGTH; ++i) {
                std::cout << messageX[i];
                if (i < LIN_SEGMENT_LENGTH - 1) std::cout << ",";
            }
            std::cout << std::endl;

            std::cout << "MessageY: ";
            for (uint8_t i = 0; i < LIN_SEGMENT_LENGTH; ++i) {
                std::cout << messageY[i];
                if (i < LIN_SEGMENT_LENGTH - 1) std::cout << ",";
            }
            std::cout << std::endl;

            std::cout << "MessageZ: ";
            for (uint8_t i = 0; i < LIN_SEGMENT_LENGTH; ++i) {
                std::cout << messageZ[i];
                if (i < LIN_SEGMENT_LENGTH - 1) std::cout << ",";
            }
            std::cout << std::endl;

        } else {
            std::cout << "Not enough data in the buffer to extract the message." << std::endl;
        }
    } else {
        std::cout << "No SyncBreak found in the buffer." << std::endl;
    }

    return 0;
}
