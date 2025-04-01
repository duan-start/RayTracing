### 整体的架构思路
1. 虽然是说用vulkan 的，但是和我们没啥关系，只是说他背后的库是这个而已
2. 这个光线追踪实际上还是用软光栅的形式写的
3. 前期准备
    - 直接使用这个模板在自己的仓库里面创建，
    - 然后clone --recursive 递归克隆到本地就好了（会把依赖项一起下好）
    - 安装vlukan sdk

4. 开始调试
    - 创建一个私有的图像的指针数据  
    - ImGui::Begin(); 
    - if(ImGui::Button("Rendering");) ...Render();
    - ImGui::End()
    - 创建一个Render函数
    - ImGui::Begin("Viewport");
    - 然后获取这个窗口的大小
    - ImGui::End();

5. Render函数
    - if(!image) 创建图片
    - uint32_t* data;存储每个像素的四个信息，4个字节
    - 0xffff00ff;从右往左，rgba，每个字节（0-255）存储颜色
    - image->setdata

6. 设置视口的图片
    - if(m_Image) Image(...)
    - 边缘填充，pushstylevar..begin之前

7. 设置时间
    Timer timer;
    timer.ElapseMills();

### 基本的数学知识
1. 解参数方程
2. 原点，方向，距离

### 渲染球体
1. 重构第一部分的代码到类Renderer