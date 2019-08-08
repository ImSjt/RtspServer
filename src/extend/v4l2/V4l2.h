#ifndef _LIBV4L2_H_
#define _LIBV4L2_H_

#include <linux/videodev2.h>
#include <stdint.h>

struct v4l2_buf_unit {
    int                index;
    void*              start;
    uint32_t           length;
    uint32_t           offset;
};

struct v4l2_buf {
    struct v4l2_buf_unit* buf;
    int nr_bufs;
    enum v4l2_buf_type type;
};



/*
 * 函数名称：v4l2_open
 * 功能描述：打开v4l2设备
 * 输入参数：name - 设备名字
 * 输入参数：flag - 打开设备标志，与open相同
 * 输出参数：无
 * 返 回 值：失败返回-1;成功返回文家描述符
 */ 
int v4l2_open(const char* name, int flag);



/*
 * 函数名称：v4l2_close
 * 功能描述：关闭v4l2设备
 * 输入参数：fd - v4l2设备的文件描述符
 * 输出参数：无
 * 返 回 值：失败返回-1;成功返回0
 */ 
int v4l2_close(int fd);



/*
 * 函数名称：v4l2_querycap
 * 功能描述：查询v4l2设备功能
 * 输入参数：fd   - v4l2设备的文件描述符
 * 输出参数：cap  - 得到的设备信息
 * 返 回 值：失败返回-1;成功返回0
 */
int v4l2_querycap(int fd, struct v4l2_capability* cap);



/*
 * 函数名称：v4l2_enuminput
 * 功能描述：枚举输入设备
 * 输入参数：fd         - v4l2设备的文件描述符
 * 输入参数：index - 输入设备的索引
 * 输出参数：name       - 返回输入设备的名称(此函数必须是一个buf)
 * 返 回 值：失败返回-1;成功返回0
 */
int v4l2_enuminput(int fd, int index, char* name);



/*
 * 函数名称：v4l2_s_input
 * 功能描述：设置输入设备
 * 输入参数：fd         - v4l2设备的文件描述符
 * 输入参数：index - 输入设备的索引
 * 输出参数：无
 * 返 回 值：失败返回-1;成功返回0
 */
int v4l2_s_input(int fd, int index);



/*
 * 函数名称：v4l2_enum_fmt
 * 功能描述：枚举设备支持的格式，判断是否支持指定的格式
 * 输入参数：fd         - v4l2设备的文件描述符
 * 输入参数：fmt        - 指定图像格式
 * 输出参数：无
 * 返 回 值：不支持返回-1;支持返回0
 *
 * fmt：
 *  V4L2_PIX_FMT_RGB565
 *  V4L2_PIX_FMT_RGB32
 *  V4L2_PIX_FMT_YUYV
 *  V4L2_PIX_FMT_UYVY
 *  V4L2_PIX_FMT_VYUY
 *  V4L2_PIX_FMT_YVYU
 *  V4L2_PIX_FMT_YUV422P
 *  V4L2_PIX_FMT_NV12
 *  V4L2_PIX_FMT_NV12T
 *  V4L2_PIX_FMT_NV21
 *  V4L2_PIX_FMT_NV16
 *  V4L2_PIX_FMT_NV61
 *  V4L2_PIX_FMT_YUV420
 *  V4L2_PIX_FMT_JPEG
 */
int v4l2_enum_fmt(int fd, unsigned int fmt, enum v4l2_buf_type type);



/*
 * 函数名称：v4l2_s_fmt
 * 功能描述：设置图像格式
 * 输入参数：fd          - v4l2设备的文件描述符
 * 输入参数：width  - 图像宽度
 * 输入参数：height - 图像高度
 * 输入参数：fmt         - 像素格式
 * 输入参数：type   - 缓存类型
 * 输出参数：width  - 修改过后的图像宽度
 * 输出参数：height - 修改过后的图像高度
 * 返 回 值：失败返回-1;成功返回0
 *
 * fmt：
 *  V4L2_PIX_FMT_RGB565
 *  V4L2_PIX_FMT_RGB32
 *  V4L2_PIX_FMT_YUYV
 *  V4L2_PIX_FMT_UYVY
 *  V4L2_PIX_FMT_VYUY
 *  V4L2_PIX_FMT_YVYU
 *  V4L2_PIX_FMT_YUV422P
 *  V4L2_PIX_FMT_NV12
 *  V4L2_PIX_FMT_NV12T
 *  V4L2_PIX_FMT_NV21
 *  V4L2_PIX_FMT_NV16
 *  V4L2_PIX_FMT_NV61
 *  V4L2_PIX_FMT_YUV420
 *  V4L2_PIX_FMT_JPEG
 *
 * type：
 *  V4L2_BUF_TYPE_VIDEO_CAPTURE
 *  V4L2_BUF_TYPE_VIDEO_OUTPUT
 *  V4L2_BUF_TYPE_VIDEO_OVERLAY
 *  V4L2_BUF_TYPE_VBI_CAPTURE
 *  V4L2_BUF_TYPE_VBI_OUTPUT
 *  V4L2_BUF_TYPE_SLICED_VBI_CAPTURE
 *  V4L2_BUF_TYPE_SLICED_VBI_OUTPUT
 *  V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY
 *  V4L2_BUF_TYPE_PRIVATE
 */
int v4l2_s_fmt(int fd, int* width, int* height, unsigned int fmt, enum v4l2_buf_type type);



/*
 * 函数名称：v4l2_reqbufs
 * 功能描述：申请缓存
 * 输入参数：fd            - v4l2设备的文件描述符
 * 输入参数：tyep          - 缓存类型
 * 输入参数：nr_bufs       - 缓存数量
 * 输出参数：无
 * 返 回 值：失败返回NULL;成功返回v4l2_buf结构体指针
 */
struct v4l2_buf* v4l2_reqbufs(int fd, enum v4l2_buf_type type, int nr_bufs);



/*
 * 函数名称：v4l2_querybuf
 * 功能描述：查询缓存信息缓存
 * 输入参数：fd            - v4l2设备的文件描述符
 * 输入参数：v4l2_buf      - v4l2_buf结构体指针
 * 输出参数：无
 * 返 回 值：失败返回-1;成功返回0
 */
int v4l2_querybuf(int fd, struct v4l2_buf* v4l2_buf);



/*
 * 函数名称：v4l2_mmap
 * 功能描述：映射缓存
 * 输入参数：fd            - v4l2设备的文件描述符
 * 输入参数：v4l2_buf      - v4l2_buf结构体指针
 * 输出参数：无
 * 返 回 值：失败返回-1;成功返回0
 */
int v4l2_mmap(int fd, struct v4l2_buf* v4l2_buf);



/*
 * 函数名称：v4l2_munmap
 * 功能描述：取消缓存映射
 * 输入参数：fd            - v4l2设备的文件描述符
 * 输入参数：v4l2_buf      - v4l2_buf结构体指针
 * 输出参数：无
 * 返 回 值：失败返回-1;成功返回0
 */
int v4l2_munmap(int fd, struct v4l2_buf* v4l2_buf);



/*
 * 函数名称：v4l2_relbufs
 * 功能描述：释放缓存
 * 输入参数：v4l2_buf      - v4l2_buf结构体指针
 * 输出参数：无
 * 返 回 值：失败返回-1;成功返回0
 */
int v4l2_relbufs(struct v4l2_buf* v4l2_buf);



/*
 * 函数名称：v4l2_streamon
 * 功能描述：开始采集
 * 输入参数：fd      - v4l2文件描述符
 * 输出参数：无
 * 返 回 值：失败返回-1;成功返回0
 */
int v4l2_streamon(int fd);



/*
 * 函数名称：v4l2_streamon
 * 功能描述：停止采集
 * 输入参数：fd      - v4l2文件描述符
 * 输出参数：无
 * 返 回 值：失败返回-1;成功返回0
 */
int v4l2_streamoff(int fd);



/*
 * 函数名称：v4l2_qbuf
 * 功能描述：缓存入队列
 * 输入参数：fd     -v4l2文件描述符
 * 输入参数：buf-缓存单元
 * 输出参数：无
 * 返 回 值：失败返回-1;成功返回0
 */
int v4l2_qbuf(int fd, struct v4l2_buf_unit* buf);



/*
 * 函数名称：v4l2_qbuf_all
 * 功能描述：所有缓存入队列
 * 输入参数：fd     -v4l2文件描述符
 * 输入参数：v4l2_buf     -缓存
 * 输出参数：无
 * 返 回 值：失败返回-1;成功返回0
 */
int v4l2_qbuf_all(int fd, struct v4l2_buf* v4l2_buf);



/*
 * 函数名称：v4l2_dqbuf
 * 功能描述：缓存出队列
 * 输入参数：fd     -v4l2文件描述符
 * 输出参数：无
 * 返 回 值：失败返回NULL;成功返回buf单元
 */
struct v4l2_buf_unit* v4l2_dqbuf(int fd, struct v4l2_buf* v4l2_buf);




/*
 * 函数名称：v4l2_g_ctrl
 * 功能描述：获取指定control信息
 * 输入参数：fd      - v4l2文件描述符
 * 输入参数：id - control id
 * 输出参数：无
 * 返 回 值：失败返回-1;成功返回control id对应的value
 */
int v4l2_g_ctrl(int fd, unsigned int id);



/*
 * 函数名称：v4l2_s_ctrl
 * 功能描述：缓存出队列
 * 输入参数：fd         - v4l2文件描述符
 * 输入参数：id         - control id
 * 输入参数：value      - control id 对应的value
 * 输出参数：无
 * 返 回 值：失败返回-1;成功返回0
 */
int v4l2_s_ctrl(int fd, unsigned int id, unsigned int value);



/*
 * 函数名称：v4l2_g_parm
 * 功能描述：获取参数
 * 输入参数：fd              - v4l2文件描述符
 * 输出参数：streamparm - 获取到的参数
 * 返 回 值：NULL-失败;非NULL - v4l2_buf_unit结构体指针
 */
int v4l2_g_parm(int fd, struct v4l2_streamparm* streamparm);



/*
 * 函数名称：v4l2_s_parm
 * 功能描述：设置参数
 * 输入参数：fd              - v4l2文件描述符
 * 输入参数：streamparm - 参数设置数据
 * 输出参数：无
 * 返 回 值：失败返回-1;成功返回0
 */
int v4l2_s_parm(int fd, struct v4l2_streamparm *streamparm);



/*
 * 函数名称：v4l2_poll
 * 功能描述：等待缓存就绪
 * 输入参数：fd      - v4l2文件描述符
 * 输出参数：无
 * 返 回 值：失败返回-1;成功返回0
 */
int v4l2_poll(int fd);

#endif //_LIBV4L2_H_
