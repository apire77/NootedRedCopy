// Copyright © 2022-2024 ChefKiss. Licensed under the Thou Shalt Not Profit License version 1.5.
// See LICENSE for details.

#pragma once
#include "ObjectField.hpp"
#include <Headers/kern_patcher.hpp>
#include <IOKit/IOService.h>
#include <IOKit/graphics/IOGraphicsTypes.h>

using t_MessageAccelerator = IOReturn (*)(void *that, UInt32 requestType, void *arg2, void *arg3, void *arg4);
using t_DceDriverSetBacklight = void (*)(void *panelCntl, UInt32 backlightPwm);
using t_DcLinkSetBacklightLevel = bool (*)(void *link, UInt32 backlightPwm, UInt32 frameRamp);
using t_DcLinkSetBacklightLevelNits = bool (*)(void *link, bool isHDR, UInt32 backlightMillinits,
    UInt32 transitionTimeMs);

class X6000FB {
    friend class NRed;

    static X6000FB *callback;

    public:
    void init();
    bool processKext(KernelPatcher &patcher, size_t id, mach_vm_address_t slide, size_t size);

    private:
    ObjectField<UInt8> dcLinkCapsField {};
    bool fixedVBIOS {false};

    UInt32 curPwmBacklightLvl {0}, maxPwmBacklightLvl {0xFFFF};
    UInt32 maxOLED {1000 * 512};
    IONotifier *dispNotif {nullptr};
    void *embeddedPanelLink {nullptr};
    bool supportsAUX {false};

    mach_vm_address_t orgSetAttributeForConnection {0};
    mach_vm_address_t orgGetAttributeForConnection {0};
    mach_vm_address_t orgGetNumberOfConnectors {0};
    mach_vm_address_t orgIH40IVRingInitHardware {0};
    mach_vm_address_t orgIRQMGRWriteRegister {0};
    t_MessageAccelerator orgMessageAccelerator {nullptr};
    mach_vm_address_t orgControllerPowerUp {0};
    mach_vm_address_t orgDpReceiverPowerCtrl {0};
    t_DceDriverSetBacklight orgDceDriverSetBacklight {nullptr};
    mach_vm_address_t orgDcePanelCntlHwInit {0};
    void *panelCntlPtr {nullptr};
    mach_vm_address_t orgLinkCreate {0};
    t_DcLinkSetBacklightLevel orgDcLinkSetBacklightLevel {0};
    t_DcLinkSetBacklightLevelNits orgDcLinkSetBacklightLevelNits {0};
    mach_vm_address_t orgInitWithPciInfo {0};

    static bool OnAppleBacklightDisplayLoad(void *target, void *refCon, IOService *newService, IONotifier *notifier);
    void registerDispMaxBrightnessNotif();

    static UInt16 wrapGetEnumeratedRevision();
    static IOReturn wrapPopulateVramInfo(void *that, void *fwInfo);
    static IOReturn wrapSetAttributeForConnection(IOService *framebuffer, IOIndex connectIndex, IOSelect attribute,
        uintptr_t value);
    static IOReturn wrapGetAttributeForConnection(IOService *framebuffer, IOIndex connectIndex, IOSelect attribute,
        uintptr_t *value);
    static UInt32 wrapGetNumberOfConnectors(void *that);
    static bool wrapIH40IVRingInitHardware(void *ctx, void *param2);
    static void wrapIRQMGRWriteRegister(void *ctx, UInt64 index, UInt32 value);
    static UInt32 wrapControllerPowerUp(void *that);
    static void wrapDpReceiverPowerCtrl(void *link, bool power_on);
    static UInt32 wrapDcePanelCntlHwInit(void *panelCntl);
    static void *wrapLinkCreate(void *data);
    static bool wrapInitWithPciInfo(void *that, void *pciDevice);
    static void wrapDoGPUPanic(char const *fmt, ...);
    static void wrapDmLoggerWrite(void *logger, const UInt32 logType, const char *fmt, ...);
};

//------ Patterns ------//

static const UInt8 kCailAsicCapsTablePattern[] = {0x6E, 0x00, 0x00, 0x00, 0x98, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};

static const UInt8 kPopulateVramInfoPattern[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x53, 0x48, 0x81, 0xEC,
    0x08, 0x01, 0x00, 0x00, 0x40, 0x89, 0xF0, 0x40, 0x89, 0xF0, 0x4C, 0x8D, 0xBD, 0xE0, 0xFE, 0xFF, 0xFF};
static const UInt8 kPopulateVramInfoMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xF0, 0xF0, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static const UInt8 kGetNumberOfConnectorsPattern[] = {0x55, 0x48, 0x89, 0xE5, 0x40, 0x8B, 0x40, 0x28, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x85, 0x00, 0x74, 0x00};
static const UInt8 kGetNumberOfConnectorsMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};

static const UInt8 kIH40IVRingInitHardwarePattern[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41,
    0x54, 0x53, 0x50, 0x40, 0x89, 0xF0, 0x49, 0x89, 0xF0, 0x40, 0x8B, 0x00, 0x00, 0x44, 0x00, 0x00};
static const UInt8 kIH40IVRingInitHardwareMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xFF, 0xF0, 0xF0, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF};

static const UInt8 kIRQMGRWriteRegisterPattern[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41,
    0x54, 0x53, 0x50, 0x41, 0x89, 0xD6, 0x49, 0x89, 0xF7, 0x48, 0x89, 0xFB, 0x48, 0x8B, 0x87, 0xB0, 0x00, 0x00, 0x00,
    0x48, 0x85, 0xC0};
static const UInt8 kIRQMGRWriteRegisterPattern14_4[] = {0x55, 0x48, 0x89, 0xe5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55,
    0x41, 0x54, 0x53, 0x50, 0x89, 0xd3, 0x49, 0x89, 0xf7, 0x49, 0x89, 0xfe, 0x48, 0x8b, 0x87, 0xb0, 0x00, 0x00, 0x00,
    0x48, 0x85, 0xc0};

static const UInt8 kDpReceiverPowerCtrl[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x54, 0x53, 0x48,
    0x83, 0xEC, 0x10, 0x89, 0xF3, 0xB0, 0x02, 0x28, 0xD8};
static const UInt8 kDpReceiverPowerCtrl14_4[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x54, 0x53, 0x48,
    0x83, 0xEC, 0x10, 0x41, 0x89, 0xF7, 0xB0, 0x02, 0x44, 0x28, 0xF8};

static const UInt8 kDcePanelCntlHwInitPattern[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41,
    0x54, 0x53, 0x50, 0x49, 0x89, 0xFD, 0x4C, 0x8D, 0x45, 0xD4, 0x41, 0xC7, 0x00, 0x00, 0x00, 0x00, 0x00};
static const UInt8 kDcePanelCntlHwInitPattern14_4[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x54, 0x53,
    0x48, 0x83, 0xEC, 0x10, 0x48, 0x89, 0xFB, 0x4C, 0x8D, 0x45, 0xDC, 0x41, 0xC7, 0x00, 0x00, 0x00, 0x00, 0x00};

static const UInt8 kDceDriverSetBacklightPattern[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41,
    0x54, 0x53, 0x50, 0x41, 0x89, 0xF0, 0x40, 0x89, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x7F, 0x08, 0x40, 0x8B, 0x40, 0x28, 0x8B, 0x70, 0x10};
static const UInt8 kDceDriverSetBacklightPatternMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xF0, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF,
    0xFF};

static const UInt8 kLinkCreatePattern[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41, 0x54, 0x53,
    0x48, 0x81, 0xEC, 0x00, 0x03, 0x00, 0x00, 0x49, 0x89, 0xFD, 0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B,
    0x00, 0x48, 0x00, 0x00, 0x00, 0xBF, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00};
static const UInt8 kLinkCreateMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
    0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00};

static const UInt8 kDcLinkSetBacklightLevelPattern[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55,
    0x41, 0x54, 0x53, 0x50, 0x41, 0x89, 0xD6, 0x41, 0x89, 0xF4};
// 14.4 has an extra `mov` operation
static const UInt8 kDcLinkSetBacklightLevelPattern14_4[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55,
    0x41, 0x54, 0x53, 0x50, 0x89, 0xD3, 0x41, 0x89, 0xF6, 0x49, 0x89, 0xFC};

static const UInt8 kDcLinkSetBacklightLevelNitsPattern[] = {0x55, 0x48, 0x89, 0xE5, 0x53, 0x50, 0x40, 0x88, 0x75, 0x00,
    0x48, 0x85, 0xFF, 0x74, 0x00};
static const UInt8 kDcLinkSetBacklightLevelNitsMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00};

static const UInt8 kDmLoggerWritePattern[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41, 0x54,
    0x53, 0x48, 0x81, 0xEC, 0x88, 0x04, 0x00, 0x00};

static const UInt8 kDalDmLoggerShouldLogPartialPattern[] = {0x48, 0x8d, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x8b, 0x04, 0x81,
    0x0f, 0xa3, 0xd0, 0x0f, 0x92, 0xc0};
static const UInt8 kDalDmLoggerShouldLogPartialPatternMask[] = {0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

//------ Patches ------//

// Fix register read (0xD31 -> 0xD2F) and family ID (0x8F -> 0x8E).
static const UInt8 kPopulateDeviceInfoOriginal[] {0xBE, 0x31, 0x0D, 0x00, 0x00, 0xFF, 0x90, 0x40, 0x01, 0x00, 0x00,
    0xC7, 0x43, 0x00, 0x8F, 0x00, 0x00, 0x00};
static const UInt8 kPopulateDeviceInfoMask[] {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF};
static const UInt8 kPopulateDeviceInfoPatched[] {0xBE, 0x2F, 0x0D, 0x00, 0x00, 0xFF, 0x90, 0x40, 0x01, 0x00, 0x00, 0xC7,
    0x43, 0x00, 0x8E, 0x00, 0x00, 0x00};

// Neutralise `AmdAtomVramInfo` creation null check.
// We don't have this entry in our VBIOS.
static const UInt8 kAmdAtomVramInfoNullCheckOriginal[] = {0x48, 0x89, 0x83, 0x90, 0x00, 0x00, 0x00, 0x48, 0x85, 0xC0,
    0x0F, 0x84, 0x89, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x7B, 0x18};
static const UInt8 kAmdAtomVramInfoNullCheckPatched[] = {0x48, 0x89, 0x83, 0x90, 0x00, 0x00, 0x00, 0x66, 0x90, 0x66,
    0x90, 0x66, 0x90, 0x66, 0x90, 0x90, 0x48, 0x8B, 0x7B, 0x18};

// Ditto
static const UInt8 kAmdAtomVramInfoNullCheckCatalinaOriginal[] = {0x48, 0x89, 0x83, 0x80, 0x00, 0x00, 0x00, 0x48, 0x85,
    0xC0, 0x74, 0x00};
static const UInt8 kAmdAtomVramInfoNullCheckCatalinaMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0x00};
static const UInt8 kAmdAtomVramInfoNullCheckCatalinaPatched[] = {0x48, 0x89, 0x83, 0x80, 0x00, 0x00, 0x00, 0x66, 0x90,
    0x66, 0x90, 0x90};

// Neutralise `AmdAtomPspDirectory` creation null check.
// We don't have this entry in our VBIOS.
static const UInt8 kAmdAtomPspDirectoryNullCheckOriginal[] = {0x48, 0x89, 0x83, 0x88, 0x00, 0x00, 0x00, 0x48, 0x85,
    0xC0, 0x0F, 0x84, 0xA1, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x7B, 0x18};
static const UInt8 kAmdAtomPspDirectoryNullCheckPatched[] = {0x48, 0x89, 0x83, 0x88, 0x00, 0x00, 0x00, 0x66, 0x90, 0x66,
    0x90, 0x66, 0x90, 0x66, 0x90, 0x90, 0x48, 0x8B, 0x7B, 0x18};

// Neutralise `AmdAtomVramInfo` null check.
static const UInt8 kGetFirmwareInfoNullCheckOriginal[] = {0x48, 0x83, 0xBB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x84,
    0x00, 0x00, 0x00, 0x00, 0x49, 0x89};
static const UInt8 kGetFirmwareInfoNullCheckOriginalMask[] = {0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF};
static const UInt8 kGetFirmwareInfoNullCheckPatched[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x90,
    0x66, 0x90, 0x66, 0x90, 0x00, 0x00};
static const UInt8 kGetFirmwareInfoNullCheckPatchedMask[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00};
static const UInt8 kGetFirmwareInfoNullCheckOriginal14_4[] = {0x49, 0x83, 0xBC, 0x24, 0x90, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x49, 0x89};
static const UInt8 kGetFirmwareInfoNullCheckOriginalMask14_4[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF};
static const UInt8 kGetFirmwareInfoNullCheckPatched14_4[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66,
    0x90, 0x66, 0x90, 0x66, 0x90, 0x00, 0x00};
static const UInt8 kGetFirmwareInfoNullCheckPatchedMask14_4[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00};

// Tell AGDC that we're an iGPU.
static const UInt8 kGetVendorInfoOriginal[] = {0x48, 0x00, 0x02, 0x10, 0x00, 0x00, 0x02};
static const UInt8 kGetVendorInfoMask[] = {0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static const UInt8 kGetVendorInfoPatched[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static const UInt8 kGetVendorInfoPatchedMask[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF};
static const UInt8 kGetVendorInfoOriginal14_4[] = {0xC7, 0x00, 0x24, 0x02, 0x10, 0x00, 0x00, 0xC7, 0x00, 0x28, 0x02,
    0x00, 0x00, 0x00};
static const UInt8 kGetVendorInfoMask14_4[] = {0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF};
static const UInt8 kGetVendorInfoPatched14_4[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00};
static const UInt8 kGetVendorInfoPatchedMask14_4[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00};

// Remove new FB count condition so we can restore the original behaviour before Ventura.
static const UInt8 kControllerPowerUpOriginal[] = {0x38, 0xC8, 0x0F, 0x42, 0xC8, 0x88, 0x8F, 0xBC, 0x00, 0x00, 0x00,
    0x72, 0x00};
static const UInt8 kControllerPowerUpOriginalMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x00};
static const UInt8 kControllerPowerUpReplace[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xEB, 0x00};
static const UInt8 kControllerPowerUpReplaceMask[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0x00};

// Remove new problematic Ventura pixel clock multiplier calculation which causes timing validation mishaps.
static const UInt8 kValidateDetailedTimingOriginal[] = {0x66, 0x0F, 0x2E, 0xC1, 0x76, 0x06, 0xF2, 0x0F, 0x5E, 0xC1};
static const UInt8 kValidateDetailedTimingPatched[] = {0x66, 0x0F, 0x2E, 0xC1, 0x66, 0x90, 0xF2, 0x0F, 0x5E, 0xC1};

// Enable all Display Core logs
static const UInt8 kInitPopulateDcInitDataOriginal[] = {0x48, 0xB9, 0xDB, 0x1B, 0xFF, 0x7E, 0x10, 0x00, 0x00, 0x00};
static const UInt8 kInitPopulateDcInitDataPatched[] = {0x48, 0xB9, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Ditto, for Catalina
static const UInt8 kInitPopulateDcInitDataCatalinaOriginal[] = {0x48, 0xC7, 0x87, 0x20, 0x02, 0x00, 0x00, 0xDB, 0x1B,
    0xFF, 0x7E};
static const UInt8 kInitPopulateDcInitDataCatalinaPatched[] = {0x48, 0xC7, 0x87, 0x20, 0x02, 0x00, 0x00, 0xFF, 0xFF,
    0xFF, 0xFF};
