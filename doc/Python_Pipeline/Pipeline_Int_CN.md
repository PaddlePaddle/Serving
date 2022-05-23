# Python Pipeline 框架

在许多深度学习框架中，模型服务化部署通常用于单模型的一键部署。但在 AI 工业大生产的背景下，端到端的单一深度学习模型不能解决复杂问题，多个深度学习模型组合使用是解决现实复杂问题的常规手段，如文字识别 OCR 服务至少需要检测和识别2种模型；视频理解服务一般需要视频抽帧、切词、音频处理、分类等多种模型组合实现。当前，通用多模型组合服务的设计和实现是非常复杂的，既要能实现复杂的模型拓扑关系，又要保证服务的高并发、高可用和易于开发和维护等。

Paddle Serving 实现了一套通用的多模型组合服务编程框架 Python Pipeline，不仅解决上述痛点，同时还能大幅提高 GPU 利用率，并易于开发和维护。

Python Pipeline 使用案例请阅读[Python Pipeline 快速部署案例](../Quick_Start_CN.md)

通过阅读以下内容掌握 Python Pipeline 核心功能和使用方法、高阶功能用法和性能优化指南等。
- [Python Pipeline 框架设计](./Pipeline_Design_CN.md)
- [Python Pipeline 核心功能](./Pipeline_Features_CN.md)
- [Python Pipeline 优化指南](./Pipeline_Optimize_CN.md)
- [Python Pipeline 性能指标](./Pipeline_Benchmark_CN.md)
