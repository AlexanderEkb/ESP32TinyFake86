//===================================================================================================
//===================================================================================================
// Glue code for Bitluni's graphics library

#include <stdint.h>
class CompositeColorOutput {
    public:
        static constexpr uint32_t XRES = 336;
        static constexpr uint32_t YRES = 240;

        void init(char ***frame);
        void sendFrameHalfResolution(char ***frame);
        void setColorburstEnabled(bool bEnabled);
        void setBlitter(uint32_t blitter);
        void saveSettings(void);
        void restoreSettings(void);
        void setPhase(uint32_t phase);
    private:
        uint32_t blitterIndex;
        uint32_t storedBlitter;
        bool storedColorburst;
};
