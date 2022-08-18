#include <spdlog/fmt/fmt.h>
#include "PerformanceGPUView.h"
#include "../../../api/GPUApi.h"
#include "../../../utils/UnitConverter.h"

PerformanceGPUView::PerformanceGPUView(MainWindow *window, PerformanceButton *button, int gpuId)
    : PerformanceSubView(window, button, new GraphWidget(0, 100, 60)) {
    m_GpuId = gpuId;

    Gdk::RGBA gpuColor;
    gpuColor.set_rgba(118 / 256., 185 / 256., 0., 1.);

    button->GetGraph()->SetAxisColor(gpuColor);
    m_UsageGraph->SetAxisColor(gpuColor);

    button->SetTitle(fmt::format("GPU {}", gpuId));
    button->SetInfoText("0%");

    m_HeadlineTitle.set_text(fmt::format("GPU {}", gpuId));
    m_HeadlineDevice.set_text(GPUApi::GetGPUName(m_GpuId));

    AddFlowDetail("Usage", m_FlowGpuUsage);
    AddFlowDetail("Speed", m_FlowGpuClock);
    AddFlowDetail("Video memory", m_FlowGpuMemory);

    AddDetail("Driver", m_DetailDriver);
    m_DetailDriver.set_text(GPUApi::GetGPUDriverString(m_GpuId));

    m_UpdateThread = new DispatcherThread([this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            m_UpdateThread->Dispatch();
        }, [this]() {
            auto gpuClockMhz = GPUApi::GetGPUClock(m_GpuId) / 1000000;
            auto gpuUsagePercent = GPUApi::GetGPUUsagePercent(m_GpuId);

            auto totalMem = GPUApi::GetGPUTotalMemory(m_GpuId);
            auto usedMem = GPUApi::GetGPUUsedMemory(m_GpuId);
            auto memUsagePercent = 100. * (double)usedMem / (double)totalMem;
            auto memUnit = UnitConverter::GetBestUnitForBytes(totalMem);

            auto gpuUsageStr = fmt::format("{:.0f}%", gpuUsagePercent);
            m_Button->SetInfoText(gpuUsageStr);
            m_FlowGpuUsage.set_text(gpuUsageStr);

            m_FlowGpuClock.set_text(fmt::format("{}MHz", gpuClockMhz));
            m_FlowGpuMemory.set_text(fmt::format("{:.1f}/{:.0f} {} ({:.0f} %)",
                                                 UnitConverter::ConvertBytes(usedMem, memUnit).value,
                                                 UnitConverter::ConvertBytes(totalMem, memUnit).value,
                                                 UnitConverter::UnitTypeToString(memUnit),
                                                 memUsagePercent));

            m_UsageGraph->AddPoint(gpuUsagePercent);
            m_Button->AddGraphPoint(gpuUsagePercent);
    });
    m_UpdateThread->Start();
}
