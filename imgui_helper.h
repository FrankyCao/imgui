#ifndef IMGUIHELPER_H_
#define IMGUIHELPER_H_

#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <immat.h>
#if IMGUI_VULKAN_SHADER
#include <imvk_gpu.h>
#endif
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

namespace ImGui {
// ImGui Info
IMGUI_API void ShowImGuiInfo();
// Image load
IMGUI_API void ImGenerateOrUpdateTexture(ImTextureID& imtexid,int width,int height,int channels,size_t offset,const unsigned char* pixels,bool useMipmapsIfPossible,bool wraps,bool wrapt,bool minFilterNearest = false,bool magFilterNearest=false,bool is_vulkan=false);
IMGUI_API inline void ImGenerateOrUpdateTexture(ImTextureID& imtexid,int width,int height,int channels,const unsigned char* pixels) { ImGenerateOrUpdateTexture(imtexid, width, height, channels, 0, pixels,false,false,false,false,false,false); };
IMGUI_API inline void ImGenerateOrUpdateTexture(ImTextureID& imtexid,int width,int height,int channels,size_t offset,const unsigned char* pixels) { ImGenerateOrUpdateTexture(imtexid, width, height, channels, offset, pixels,false,false,false,false,false,true); };
IMGUI_API ImTextureID ImCreateTexture(const void* data, int width, int height, double time_stamp = NAN);
IMGUI_API ImTextureID ImLoadTexture(const char* path);
IMGUI_API void ImDestroyTexture(ImTextureID texture);
IMGUI_API int ImGetTextureWidth(ImTextureID texture);
IMGUI_API int ImGetTextureHeight(ImTextureID texture);
IMGUI_API double ImGetTextureTimeStamp(ImTextureID texture);
IMGUI_API bool ImTextureToFile(ImTextureID texture, std::string path);
IMGUI_API void ImMatToTexture(ImGui::ImMat mat, ImTextureID& texture);
#if IMGUI_RENDERING_VULKAN && IMGUI_VULKAN_SHADER
IMGUI_API ImTextureID ImCreateTexture(ImGui::VkImageMat & image, double time_stamp = NAN);
#endif
// Experimental: tested on Ubuntu only. Should work with urls, folders and files.
IMGUI_API bool OpenWithDefaultApplication(const char* url,bool exploreModeForWindowsOS=false);

IMGUI_API void CloseAllPopupMenus();  // Never Tested

IMGUI_API bool IsItemActiveLastFrame();
IMGUI_API bool IsItemJustReleased();

IMGUI_API void ShowTooltipOnHover(const char* fmt, ...) IM_FMTARGS(1);
IMGUI_API void ShowTooltipOnHoverV(const char* fmt, va_list args) IM_FMTLIST(1);

// Drawn an rectangle around last ImGui widget.
IMGUI_API void Debug_DrawItemRect(const ImVec4& col = ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

IMGUI_API const ImFont* GetFont(int fntIndex);
IMGUI_API void PushFont(int fntIndex);    // using the index of the font instead of a ImFont* is easier (you can set up an enum).
IMGUI_API void TextColoredV(int fntIndex,const ImVec4& col, const char* fmt, va_list args);
IMGUI_API void TextColored(int fntIndex,const ImVec4& col, const char* fmt, ...) IM_FMTARGS(3);
IMGUI_API void TextV(int fntIndex,const char* fmt, va_list args);
IMGUI_API void Text(int fntIndex,const char* fmt, ...) IM_FMTARGS(2);
IMGUI_API void TextComplex(const char * str, float scale, ImVec4 text_color, float outline_w, ImVec4 outline_color, ImVec2 shadow_offset, ImVec4 shadow_color);
// Handy if we want to use ImGui::Image(...) or ImGui::ImageButton(...) with a glyph
IMGUI_API bool GetTexCoordsFromGlyph(unsigned short glyph,ImVec2& uv0,ImVec2& uv1);
// Returns the height of the main menu based on the current font and style
// Warning: according to https://github.com/ocornut/imgui/issues/252 this approach can fail [Better call ImGui::GetWindowSize().y from inside the menu and store the result somewhere]
IMGUI_API float CalcMainMenuHeight();

// These two methods are inspired by imguidock.cpp
// if optionalRootWindowName==NULL, they refer to the current window
// P.S. This methods are never used anywhere, and it's not clear to me when
// PutInForeground() is better then ImGui::SetWindowFocus()
IMGUI_API void PutInBackground(const char* optionalRootWindowName=NULL);
IMGUI_API void PutInForeground(const char* optionalRootWindowName=NULL);

// ImGui Stringify
IMGUI_API bool Base64Encode(const char* input,int inputSize,ImVector<char>& output,bool stringifiedMode=false,int numCharsPerLineInStringifiedMode=112);
IMGUI_API bool Base64Decode(const char* input,ImVector<char>& output);

IMGUI_API bool Base85Encode(const char* input,int inputSize,ImVector<char>& output,bool stringifiedMode=false,int numCharsPerLineInStringifiedMode=112);
IMGUI_API bool Base85Decode(const char* input,ImVector<char>& output);

IMGUI_API bool BinaryStringify(const char* input, int inputSize, ImVector<char>& output, int numInputBytesPerLineInStringifiedMode=80, bool serializeUnsignedBytes=false);
IMGUI_API bool TextStringify(const char* input, ImVector<char>& output, int numCharsPerLineInStringifiedMode=0, int inputSize=0, bool noBackslashAtLineEnds=false);
// Two methods that fill rv and return true on success
IMGUI_API bool Base64DecodeFromFile(const char* filePath,ImVector<char>& rv);
IMGUI_API bool Base85DecodeFromFile(const char* filePath,ImVector<char>& rv);

// FFT 1D
IMGUI_API void ImFFT (float* data, int N, bool forward);
IMGUI_API void ImRFFT (float* data, int N, bool forward);
IMGUI_API void ImRFFT (float* in, float* out, int N, bool forward);
IMGUI_API int ImReComposeDB(float * in, float * out, int samples, bool inverse = true);
IMGUI_API int ImReComposeAmplitude(float * in, float * out, int samples);
IMGUI_API int ImReComposePhase(float * in, float * out, int samples);
IMGUI_API int ImReComposeDBShort(float * in, float * out, int samples, bool inverse = true);
IMGUI_API int ImReComposeDBLong(float * in, float * out, int samples, bool inverse = true);
IMGUI_API float ImDoDecibel(float * in, int samples, bool inverse = true);
// STFT 1D
struct IMGUI_API ImSTFT
{
    ImSTFT(int _window, int _hope);
    ~ImSTFT();
    void stft(float* in, float* out);
    void istft(float* in, float* out);

private:
    void *hannwin {nullptr};
    void *overlap {nullptr};

    int frame_size;
    int shift_size;
    int overlap_size;
    float* buf  {nullptr};
};

#ifdef IMGUI_USE_ZLIB	// requires linking to library -lZlib
// Two methods that fill rv and return true on success
IMGUI_API bool GzDecompressFromFile(const char* filePath,ImVector<char>& rv,bool clearRvBeforeUsage=true);
IMGUI_API bool GzBase64DecompressFromFile(const char* filePath,ImVector<char>& rv);
IMGUI_API bool GzBase85DecompressFromFile(const char* filePath,ImVector<char>& rv);
IMGUI_API bool GzDecompressFromMemory(const char* memoryBuffer,int memoryBufferSize,ImVector<char>& rv,bool clearRvBeforeUsage=true);
IMGUI_API bool GzCompressFromMemory(const char* memoryBuffer,int memoryBufferSize,ImVector<char>& rv,bool clearRvBeforeUsage=true);
IMGUI_API bool GzBase64DecompressFromMemory(const char* input,ImVector<char>& rv);
IMGUI_API bool GzBase85DecompressFromMemory(const char* input,ImVector<char>& rv);
IMGUI_API bool GzBase64CompressFromMemory(const char* input,int inputSize,ImVector<char>& output,bool stringifiedMode=false,int numCharsPerLineInStringifiedMode=112);
IMGUI_API bool GzBase85CompressFromMemory(const char* input,int inputSize,ImVector<char>& output,bool stringifiedMode=false,int numCharsPerLineInStringifiedMode=112);
#endif //IMGUI_USE_ZLIB

// IMPORTANT: FT_INT,FT_UNSIGNED,FT_FLOAT,FT_DOUBLE,FT_BOOL support from 1 to 4 components.
enum FieldType {
    FT_INT=0,
    FT_UNSIGNED,
    FT_FLOAT,
    FT_DOUBLE,
    //--------------- End types that support 1 to 4 array components ----------
    FT_STRING,      // an arbitrary-length string (or a char blob that can be used as custom type)
    FT_ENUM,        // serialized/deserialized as FT_INT
    FT_BOOL,
    FT_COLOR,       // serialized/deserialized as FT_FLOAT (with 3 or 4 components)
    FT_TEXTLINE,    // a (series of) text line(s) (separated by '\n') that are fed one at a time in the Deserializer callback
    FT_CUSTOM,      // a custom type that is served like FT_TEXTLINE (=one line at a time).
    FT_COUNT
};

struct IMGUI_API ScopedItemWidth
{
    ScopedItemWidth(float width);
    ~ScopedItemWidth();

    void Release();

private:
    bool m_IsDone = false;
};

struct IMGUI_API ScopedDisableItem
{
    ScopedDisableItem(bool disable, float disabledAlpha = 0.5f);
    ~ScopedDisableItem();

    void Release();

private:
    bool    m_Disable       = false;
    float   m_LastAlpha     = 1.0f;
};

struct IMGUI_API ScopedSuspendLayout
{
    ScopedSuspendLayout();
    ~ScopedSuspendLayout();

    void Release();

private:
    ImGuiWindow* m_Window = nullptr;
    ImVec2 m_CursorPos;
    ImVec2 m_CursorPosPrevLine;
    ImVec2 m_CursorMaxPos;
    ImVec2 m_CurrLineSize;
    ImVec2 m_PrevLineSize;
    float  m_CurrLineTextBaseOffset;
    float  m_PrevLineTextBaseOffset;
};

struct IMGUI_API ItemBackgroundRenderer
{
    using OnDrawCallback = std::function<void(ImDrawList* drawList)>;

    ItemBackgroundRenderer(OnDrawCallback onDrawBackground);
    ~ItemBackgroundRenderer();

    void Commit();
    void Discard();

private:
    ImDrawList*         m_DrawList = nullptr;
    ImDrawListSplitter  m_Splitter;
    OnDrawCallback      m_OnDrawBackground;
};

template <typename Settings>
struct StorageHandler
{
    using Storage = std::map<std::string, std::unique_ptr<Settings>>;

    ImGuiSettingsHandler MakeHandler(const char* const typeName)
    {
        ImGuiSettingsHandler handler;
        handler.TypeName = typeName;
        handler.TypeHash = ImHashStr(typeName);
        handler.UserData = this;
        handler.ReadOpenFn = [](ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* name) -> void*
        {
            auto storage = reinterpret_cast<StorageHandler*>(handler->UserData);
            return storage->DoReadOpen(ctx, name);
        };
        handler.ReadLineFn = [](ImGuiContext* ctx, ImGuiSettingsHandler* handler, void* entry, const char* line) -> void
        {
            auto storage = reinterpret_cast<StorageHandler*>(handler->UserData);
            storage->DoReadLine(ctx, reinterpret_cast<Settings*>(entry), line);
        };
        handler.WriteAllFn = [](ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* out_buf)
        {
            auto storage = reinterpret_cast<StorageHandler*>(handler->UserData);
            storage->DoWriteAll(ctx, out_buf);
        };
        return handler;
    }

    const Settings* Find(const char* name) const
    {
        auto it = m_Settings.find(name);
        if (it == m_Settings.end())
            return nullptr;

        return it->second.get();
    }

    Settings* Find(const char* name)
    {
        return const_cast<Settings*>(const_cast<const StorageHandler*>(this)->Find(name));
    }

    Settings* FindOrCreate(const char* name)
    {
        auto settings = Find(name);
        if (settings == nullptr)
        {
            auto it = m_Settings.emplace(name, std::make_unique<Settings>());
            settings = it.first->second.get();
        }
        return settings;
    }

    std::function<void(ImGuiContext*, Settings*, const char*)>              ReadOpen;
    std::function<void(ImGuiContext*, Settings*, const char*)>              ReadLine;
    std::function<void(ImGuiContext*, ImGuiTextBuffer*, const Storage&)>    WriteAll;

private:
    Settings* DoReadOpen(ImGuiContext* ctx, const char* name)
    {
        auto settings = FindOrCreate(name);
        if (ReadOpen)
            ReadOpen(ctx, settings, name);
        return settings;
    }

    void DoReadLine(ImGuiContext* ctx, Settings* entry, const char* line)
    {
        if (ReadLine)
            ReadLine(ctx, entry, line);
    }

    void DoWriteAll(ImGuiContext* ctx, ImGuiTextBuffer* out_buf)
    {
        if (WriteAll)
            WriteAll(ctx, out_buf, m_Settings);
    }

    std::map<std::string, std::unique_ptr<Settings>> m_Settings;
};

struct IMGUI_API MostRecentlyUsedList
{
    static void Install(ImGuiContext* context);

    MostRecentlyUsedList(const char* id, int capacity = 10);

    void Add(const std::string& item);
    void Add(const char* item);
    void Clear();

    const std::vector<std::string>& GetList() const;

    int Size() const;

private:
    struct Settings
    {
        std::vector<std::string> m_List;
    };

    void PullFromStorage();
    void PushToStorage();

    std::string                 m_ID;
    int                         m_Capacity = 10;
    std::vector<std::string>&   m_List;

    static StorageHandler<Settings> s_Storage;
};

struct IMGUI_API Grid
{
    void Begin(const char* id, int columns, float width = -1.0f);
    void Begin(ImU32 id, int columns, float width = -1.0f);
    void NextColumn();
    void NextRow();
    void SetColumnAlignment(float alignment);
    void End();

private:
    int Seed(int column, int row) const { return column + row * m_Columns; }
    int Seed() const { return Seed(m_Column, m_Row); }
    int ColumnSeed(int column) const { return Seed(column, -1); }
    int ColumnSeed() const { return ColumnSeed(m_Column); }

    void EnterCell(int column, int row);
    void LeaveCell();

    int m_Columns = 1;
    int m_Row = 0;
    int m_Column = 0;
    float m_MinimumWidth = -1.0f;

    ImVec2 m_CursorPos;

    ImGuiStorage* m_Storage = nullptr;

    float m_ColumnAlignment = 0.0f;
    float m_MaximumColumnWidthAcc = -1.0f;
};

struct IMGUI_API ImTree
{
    std::string name;
    std::vector<ImTree> childrens;
    void * data {nullptr};
    ImTree() {}
    ImTree(std::string _name, void * _data = nullptr) { name = _name; data = _data; }
    ImTree* FindChildren(std::string _name)
    {
        auto iter = std::find_if(childrens.begin(), childrens.end(), [_name](const ImTree& tree)
        {
            return tree.name.compare(_name) == 0;
        });
        if (iter != childrens.end())
            return &(*iter);
        else
            return nullptr;
    }
};
}   // ImGui

namespace ImGui
{
IMGUI_API double    get_current_time();
IMGUI_API uint32_t  get_current_time_msec();
IMGUI_API uint64_t  get_current_time_usec();
IMGUI_API void      sleep(float seconds);
IMGUI_API void      sleep(int ms_seconds);
} // namespace ImGui

namespace ImGui
{
// Power saving mode
// Disabled by default; enabled by setting ImGuiConfigFlags_EnablePowerSavingMode or ImGuiConfigFlags_EnableLowRefreshMode in ImGuiIO.ConfigFlags.
// Requires platform binding support.
// When enabled and supported, ImGui will wait for input events before starting new frames, instead of continuously polling, thereby helping to reduce power consumption.
// It will wake up periodically if a widget is animating (e.g. blinking InputText cursor). You can control this maximum wake-up timeout using SetMaxWaitBeforeNextFrame(), for example when your application is playing an animation.
// This wake-up/timeout event is disabled, and ImGui will wait for an input event, as long as the window is known, for sure, to be hidden. This depends on the platform binding, and does not work in all cases (e.g. if the window is in a logical/system 'visible' state, but currently sitting behind another, non-transparent window).
IMGUI_API double    GetEventWaitingTime();                      // in seconds; note that it can be zero (in which case you might want to peek/poll) or infinity (in which case you may have to use a non-timeout event waiting method).
IMGUI_API void      SetMaxWaitBeforeNextFrame(double time);     // in seconds
} // namespace ImGui

namespace ImGui
{
class IMGUI_API ImKalman
{
public:
    ImKalman(int state_size, int mea_size);
    ~ImKalman() {};

public:
    void covariance(float noise_covariance, float measurement_noise_covariance);
    void update(ImMat& Y);
    ImMat& predicted();

public:
    ImMat statePre;            //预测状态矩阵(x'(k)) x(k) = A*x(k - 1) + B * u(k)
    ImMat statePost;           //状态估计修正矩阵(x(k)) x(k) = x'(k) + K(k)*(z(k) - H * x'(k)) ： 1 * 8
    ImMat transitionMatrix;    //转移矩阵(A)  ： 8 * 8
    ImMat controMatrix;        //控制矩阵(B)
    ImMat measurementMatrix;   //测量矩阵(H) ：4 * 8
    ImMat processNoiseCov;     //预测模型噪声协方差矩阵(Q) ：8 * 8
    ImMat measurementNoiseCov; //测量噪声协方差矩阵(R)  ： 4 * 4
    ImMat errorCovPre;         //转移噪声矩阵(P'(k)) p'(k) = A * p(k - 1) * At + Q 
    ImMat K;                   //kalman增益矩阵 K = p'(k) * Ht * inv(H * p'(k) * Ht + R)
    ImMat errorCovPost;        //转移噪声修正矩阵(p(k)) p(k) = (I - K(k) * H) * p'(k)  ： 8 * 8
};
} // namespace ImGui

// These classed are supposed to be used internally
namespace ImGuiHelper {
typedef ImGui::FieldType FieldType;
// System Toolkit
IMGUI_API bool GetFileContent(const char* filePath,ImVector<char>& contentOut,bool clearContentOutBeforeUsage=true,const char* modes="rb",bool appendTrailingZeroIfModesIsNotBinary=true);
IMGUI_API bool SetFileContent(const char *filePath, const unsigned char* content, int contentSize,const char* modes="wb");

// true of file exists
IMGUI_API bool file_exists(const std::string& path);
// extract the path of a filename from a full URI (e.g. file:://home/me/toto.mpg -> file:://home/me/)
IMGUI_API std::string path_filename(const std::string& path);
// get fixed length string (17 chars) YYYYMMDDHHmmssiii
IMGUI_API std::string date_time_string();
// get the OS dependent username
IMGUI_API std::string username();
// get the OS dependent home path
IMGUI_API std::string home_path();
// get the current working directory
IMGUI_API std::string cwd_path();
// get the current exec directory
IMGUI_API std::string exec_path();
// create directory and return true on success
IMGUI_API bool create_directory(const std::string& path);
// get the OS dependent path where to store settings
IMGUI_API std::string settings_path(std::string app_name);
// get the OS dependent path where to store temporary files
IMGUI_API std::string temp_path();
// try to execute a command
IMGUI_API void execute(const std::string& command);
// return memory used (in bytes)
IMGUI_API size_t memory_usage();
// return maximum memory resident set size used (in bytes)
IMGUI_API size_t memory_max_usage();

class IMGUI_API Deserializer {
    char* f_data;
    size_t f_size;
    void clear();
    bool loadFromFile(const char* filename);
    bool allocate(size_t sizeToAllocate,const char* optionalTextToCopy=NULL,size_t optionalTextToCopySize=0);
    public:
    IMGUI_API Deserializer() : f_data(NULL),f_size(0) {}
    IMGUI_API Deserializer(const char* filename);                     // From file
    IMGUI_API Deserializer(const char* text,size_t textSizeInBytes);  // From memory (and optionally from file through GetFileContent(...))
    IMGUI_API ~Deserializer() {clear();}
    IMGUI_API bool isValid() const {return (f_data && f_size>0);}

    // returns whether to stop parsing or not
    typedef bool (*ParseCallback)(FieldType ft,int numArrayElements,void* pValue,const char* name,void* userPtr);   // (*)
    // returns a pointer to "next_line" if the callback has stopped parsing or NULL.
    // returned value can be refeed as optionalBufferStart
    const char *parse(ParseCallback cb,void* userPtr,const char* optionalBufferStart=NULL) const;

    // (*)
    /*
    FT_CUSTOM and FT_TEXTLINE are served multiple times (one per text line) with numArrayElements that goes from 0 to numTextLines-1.
    All the other field types are served once.
    */

protected:
    void operator=(const Deserializer&) {}
    Deserializer(const Deserializer&) {}
};

class ISerializable;
class Serializer {

    ISerializable* f;
    void clear();

    public:
    IMGUI_API Serializer(const char* filename);               // To file
    IMGUI_API Serializer(int memoryBufferCapacity=2048);      // To memory (and optionally to file through WriteBufferToFile(...))
    IMGUI_API ~Serializer();
    bool isValid() const {return (f);}

    IMGUI_API bool save(FieldType ft, const float* pValue, const char* name, int numArrayElements=1,int prec=3);
    IMGUI_API bool save(FieldType ft, const int* pValue, const char* name, int numArrayElements=1,int prec=-1);
    bool save(const float* pValue,const char* name,int numArrayElements=1,int prec=3)    {
        return save(ImGui::FT_FLOAT,pValue,name,numArrayElements,prec);
    }
    bool save(const int* pValue,const char* name,int numArrayElements=1,int prec=-1)  {
        return save(ImGui::FT_INT,pValue,name,numArrayElements,prec);
    }
    IMGUI_API bool save(const char* pValue,const char* name,int pValueSize=-1);
    IMGUI_API bool save(const unsigned* pValue, const char* name, int numArrayElements=1,int prec=-1);
    IMGUI_API bool save(const double* pValue, const char* name, int numArrayElements=1,int prec=-1);
    IMGUI_API bool save(const bool* pValue, const char* name, int numArrayElements=1);
    IMGUI_API bool saveTextLines(const char* pValue,const char* name); // Splits the string into N lines: each line is passed by the deserializer into a single element in the callback
    IMGUI_API bool saveTextLines(int numValues,bool (*items_getter)(void* data, int idx, const char** out_text),void* data,const char* name);

    // To serialize FT_CUSTOM:
    IMGUI_API bool saveCustomFieldTypeHeader(const char* name, int numTextLines=1); //e.g. for 4 lines "[CUSTOM-4:MyCustomFieldTypeName]\n". Then add 4 lines using getPointer() below.

    // These 2 are only available when this class is constructed with the
    // Serializer(int memoryBufferCapacity) constructor
    IMGUI_API const char* getBuffer() const;
    IMGUI_API int getBufferSize() const;
    IMGUI_API static bool WriteBufferToFile(const char* filename, const char* buffer, int bufferSize);

protected:
    void operator=(const Serializer&) {}
    Serializer(const Serializer&) {}

};

// Optional String Helper methods:
// "destText" must be released with ImGui::MemFree(destText). It should always work.
IMGUI_API void StringSet(char*& destText,const char* text,bool allowNullDestText=true);
// "destText" must be released with ImGui::MemFree(destText). It should always work.
IMGUI_API void StringAppend(char*& destText, const char* textToAppend, bool allowNullDestText=true, bool prependLineFeedIfDestTextIsNotEmpty = true, bool mustAppendLineFeed = false);
// Appends a formatted string to a char vector (= no need to free memory)
// v can't be empty (it must at least be: v.size()==1 && v[0]=='\0')
// returns the number of chars appended.
IMGUI_API int StringAppend(ImVector<char>& v,const char* fmt, ...);

// ImGui Theme generator
IMGUI_API void ThemeGenerator(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0);

} // ImGuiHelper

#ifndef NO_IMGUIKNOWNCOLOR_DEFINITIONS
#define KNOWNIMGUICOLOR_ALICEBLUE IM_COL32(240,248,255,255)
#define KNOWNIMGUICOLOR_ANTIQUEWHITE IM_COL32(250,235,215,255)
#define KNOWNIMGUICOLOR_AQUA IM_COL32(0,255,255,255)
#define KNOWNIMGUICOLOR_AQUAMARINE IM_COL32(127,255,212,255)
#define KNOWNIMGUICOLOR_AZURE IM_COL32(240,255,255,255)
#define KNOWNIMGUICOLOR_BEIGE IM_COL32(245,245,220,255)
#define KNOWNIMGUICOLOR_BISQUE IM_COL32(255,228,196,255)
#define KNOWNIMGUICOLOR_BLACK IM_COL32(0,0,0,255)
#define KNOWNIMGUICOLOR_BLANCHEDALMOND IM_COL32(255,235,205,255)
#define KNOWNIMGUICOLOR_BLUE IM_COL32(0,0,255,255)
#define KNOWNIMGUICOLOR_BLUEVIOLET IM_COL32(138,43,226,255)
#define KNOWNIMGUICOLOR_BROWN IM_COL32(165,42,42,255)
#define KNOWNIMGUICOLOR_BURLYWOOD IM_COL32(222,184,135,255)
#define KNOWNIMGUICOLOR_CADETBLUE IM_COL32(95,158,160,255)
#define KNOWNIMGUICOLOR_CHARTREUSE IM_COL32(127,255,0,255)
#define KNOWNIMGUICOLOR_CHOCOLATE IM_COL32(210,105,30,255)
#define KNOWNIMGUICOLOR_CORAL IM_COL32(255,127,80,255)
#define KNOWNIMGUICOLOR_CORNFLOWERBLUE IM_COL32(100,149,237,255)
#define KNOWNIMGUICOLOR_CORNSILK IM_COL32(255,248,220,255)
#define KNOWNIMGUICOLOR_CRIMSON IM_COL32(220,20,60,255)
#define KNOWNIMGUICOLOR_CYAN IM_COL32(0,255,255,255)
#define KNOWNIMGUICOLOR_DARKBLUE IM_COL32(0,0,139,255)
#define KNOWNIMGUICOLOR_DARKCYAN IM_COL32(0,139,139,255)
#define KNOWNIMGUICOLOR_DARKGOLDENROD IM_COL32(184,134,11,255)
#define KNOWNIMGUICOLOR_DARKGRAY IM_COL32(169,169,169,255)
#define KNOWNIMGUICOLOR_DARKGREEN IM_COL32(0,100,0,255)
#define KNOWNIMGUICOLOR_DARKKHAKI IM_COL32(189,183,107,255)
#define KNOWNIMGUICOLOR_DARKMAGENTA IM_COL32(139,0,139,255)
#define KNOWNIMGUICOLOR_DARKOLIVEGREEN IM_COL32(85,107,47,255)
#define KNOWNIMGUICOLOR_DARKORANGE IM_COL32(255,140,0,255)
#define KNOWNIMGUICOLOR_DARKORCHID IM_COL32(153,50,204,255)
#define KNOWNIMGUICOLOR_DARKRED IM_COL32(139,0,0,255)
#define KNOWNIMGUICOLOR_DARKSALMON IM_COL32(233,150,122,255)
#define KNOWNIMGUICOLOR_DARKSEAGREEN IM_COL32(143,188,139,255)
#define KNOWNIMGUICOLOR_DARKSLATEBLUE IM_COL32(72,61,139,255)
#define KNOWNIMGUICOLOR_DARKSLATEGRAY IM_COL32(47,79,79,255)
#define KNOWNIMGUICOLOR_DARKTURQUOISE IM_COL32(0,206,209,255)
#define KNOWNIMGUICOLOR_DARKVIOLET IM_COL32(148,0,211,255)
#define KNOWNIMGUICOLOR_DEEPPINK IM_COL32(255,20,147,255)
#define KNOWNIMGUICOLOR_DEEPSKYBLUE IM_COL32(0,191,255,255)
#define KNOWNIMGUICOLOR_DIMGRAY IM_COL32(105,105,105,255)
#define KNOWNIMGUICOLOR_DODGERBLUE IM_COL32(30,144,255,255)
#define KNOWNIMGUICOLOR_FIREBRICK IM_COL32(178,34,34,255)
#define KNOWNIMGUICOLOR_FLORALWHITE IM_COL32(255,250,240,255)
#define KNOWNIMGUICOLOR_FORESTGREEN IM_COL32(34,139,34,255)
#define KNOWNIMGUICOLOR_FUCHSIA IM_COL32(255,0,255,255)
#define KNOWNIMGUICOLOR_GAINSBORO IM_COL32(220,220,220,255)
#define KNOWNIMGUICOLOR_GHOSTWHITE IM_COL32(248,248,255,255)
#define KNOWNIMGUICOLOR_GOLD IM_COL32(255,215,0,255)
#define KNOWNIMGUICOLOR_GOLDENROD IM_COL32(218,165,32,255)
#define KNOWNIMGUICOLOR_GRAY IM_COL32(128,128,128,255)
#define KNOWNIMGUICOLOR_GREEN IM_COL32(0,128,0,255)
#define KNOWNIMGUICOLOR_GREENYELLOW IM_COL32(173,255,47,255)
#define KNOWNIMGUICOLOR_HONEYDEW IM_COL32(240,255,240,255)
#define KNOWNIMGUICOLOR_HOTPINK IM_COL32(255,105,180,255)
#define KNOWNIMGUICOLOR_INDIANRED IM_COL32(205,92,92,255)
#define KNOWNIMGUICOLOR_INDIGO IM_COL32(75,0,130,255)
#define KNOWNIMGUICOLOR_IVORY IM_COL32(255,255,240,255)
#define KNOWNIMGUICOLOR_KHAKI IM_COL32(240,230,140,255)
#define KNOWNIMGUICOLOR_LAVENDER IM_COL32(230,230,250,255)
#define KNOWNIMGUICOLOR_LAVENDERBLUSH IM_COL32(255,240,245,255)
#define KNOWNIMGUICOLOR_LAWNGREEN IM_COL32(124,252,0,255)
#define KNOWNIMGUICOLOR_LEMONCHIFFON IM_COL32(255,250,205,255)
#define KNOWNIMGUICOLOR_LIGHTBLUE IM_COL32(173,216,230,255)
#define KNOWNIMGUICOLOR_LIGHTCORAL IM_COL32(240,128,128,255)
#define KNOWNIMGUICOLOR_LIGHTCYAN IM_COL32(224,255,255,255)
#define KNOWNIMGUICOLOR_LIGHTGOLDENRODYELLOW IM_COL32(250,250,210,255)
#define KNOWNIMGUICOLOR_LIGHTGRAY IM_COL32(211,211,211,255)
#define KNOWNIMGUICOLOR_LIGHTGREEN IM_COL32(144,238,144,255)
#define KNOWNIMGUICOLOR_LIGHTPINK IM_COL32(255,182,193,255)
#define KNOWNIMGUICOLOR_LIGHTSALMON IM_COL32(255,160,122,255)
#define KNOWNIMGUICOLOR_LIGHTSEAGREEN IM_COL32(32,178,170,255)
#define KNOWNIMGUICOLOR_LIGHTSKYBLUE IM_COL32(135,206,250,255)
#define KNOWNIMGUICOLOR_LIGHTSLATEGRAY IM_COL32(119,136,153,255)
#define KNOWNIMGUICOLOR_LIGHTSTEELBLUE IM_COL32(176,196,222,255)
#define KNOWNIMGUICOLOR_LIGHTYELLOW IM_COL32(255,255,224,255)
#define KNOWNIMGUICOLOR_LIME IM_COL32(0,255,0,255)
#define KNOWNIMGUICOLOR_LIMEGREEN IM_COL32(50,205,50,255)
#define KNOWNIMGUICOLOR_LINEN IM_COL32(250,240,230,255)
#define KNOWNIMGUICOLOR_MAGENTA IM_COL32(255,0,255,255)
#define KNOWNIMGUICOLOR_MAROON IM_COL32(128,0,0,255)
#define KNOWNIMGUICOLOR_MEDIUMAQUAMARINE IM_COL32(102,205,170,255)
#define KNOWNIMGUICOLOR_MEDIUMBLUE IM_COL32(0,0,205,255)
#define KNOWNIMGUICOLOR_MEDIUMORCHID IM_COL32(186,85,211,255)
#define KNOWNIMGUICOLOR_MEDIUMPURPLE IM_COL32(147,112,219,255)
#define KNOWNIMGUICOLOR_MEDIUMSEAGREEN IM_COL32(60,179,113,255)
#define KNOWNIMGUICOLOR_MEDIUMSLATEBLUE IM_COL32(123,104,238,255)
#define KNOWNIMGUICOLOR_MEDIUMSPRINGGREEN IM_COL32(0,250,154,255)
#define KNOWNIMGUICOLOR_MEDIUMTURQUOISE IM_COL32(72,209,204,255)
#define KNOWNIMGUICOLOR_MEDIUMVIOLETRED IM_COL32(199,21,133,255)
#define KNOWNIMGUICOLOR_MIDNIGHTBLUE IM_COL32(25,25,112,255)
#define KNOWNIMGUICOLOR_MINTCREAM IM_COL32(245,255,250,255)
#define KNOWNIMGUICOLOR_MISTYROSE IM_COL32(255,228,225,255)
#define KNOWNIMGUICOLOR_MOCCASIN IM_COL32(255,228,181,255)
#define KNOWNIMGUICOLOR_NAVAJOWHITE IM_COL32(255,222,173,255)
#define KNOWNIMGUICOLOR_NAVY IM_COL32(0,0,128,255)
#define KNOWNIMGUICOLOR_OLDLACE IM_COL32(253,245,230,255)
#define KNOWNIMGUICOLOR_OLIVE IM_COL32(128,128,0,255)
#define KNOWNIMGUICOLOR_OLIVEDRAB IM_COL32(107,142,35,255)
#define KNOWNIMGUICOLOR_ORANGE IM_COL32(255,165,0,255)
#define KNOWNIMGUICOLOR_ORANGERED IM_COL32(255,69,0,255)
#define KNOWNIMGUICOLOR_ORCHID IM_COL32(218,112,214,255)
#define KNOWNIMGUICOLOR_PALEGOLDENROD IM_COL32(238,232,170,255)
#define KNOWNIMGUICOLOR_PALEGREEN IM_COL32(152,251,152,255)
#define KNOWNIMGUICOLOR_PALETURQUOISE IM_COL32(175,238,238,255)
#define KNOWNIMGUICOLOR_PALEVIOLETRED IM_COL32(219,112,147,255)
#define KNOWNIMGUICOLOR_PAPAYAWHIP IM_COL32(255,239,213,255)
#define KNOWNIMGUICOLOR_PEACHPUFF IM_COL32(255,218,185,255)
#define KNOWNIMGUICOLOR_PERU IM_COL32(205,133,63,255)
#define KNOWNIMGUICOLOR_PINK IM_COL32(255,192,203,255)
#define KNOWNIMGUICOLOR_PLUM IM_COL32(221,160,221,255)
#define KNOWNIMGUICOLOR_POWDERBLUE IM_COL32(176,224,230,255)
#define KNOWNIMGUICOLOR_PURPLE IM_COL32(128,0,128,255)
#define KNOWNIMGUICOLOR_RED IM_COL32(255,0,0,255)
#define KNOWNIMGUICOLOR_ROSYBROWN IM_COL32(188,143,143,255)
#define KNOWNIMGUICOLOR_ROYALBLUE IM_COL32(65,105,225,255)
#define KNOWNIMGUICOLOR_SADDLEBROWN IM_COL32(139,69,19,255)
#define KNOWNIMGUICOLOR_SALMON IM_COL32(250,128,114,255)
#define KNOWNIMGUICOLOR_SANDYBROWN IM_COL32(244,164,96,255)
#define KNOWNIMGUICOLOR_SEAGREEN IM_COL32(46,139,87,255)
#define KNOWNIMGUICOLOR_SEASHELL IM_COL32(255,245,238,255)
#define KNOWNIMGUICOLOR_SIENNA IM_COL32(160,82,45,255)
#define KNOWNIMGUICOLOR_SILVER IM_COL32(192,192,192,255)
#define KNOWNIMGUICOLOR_SKYBLUE IM_COL32(135,206,235,255)
#define KNOWNIMGUICOLOR_SLATEBLUE IM_COL32(106,90,205,255)
#define KNOWNIMGUICOLOR_SLATEGRAY IM_COL32(112,128,144,255)
#define KNOWNIMGUICOLOR_SNOW IM_COL32(255,250,250,255)
#define KNOWNIMGUICOLOR_SPRINGGREEN IM_COL32(0,255,127,255)
#define KNOWNIMGUICOLOR_STEELBLUE IM_COL32(70,130,180,255)
#define KNOWNIMGUICOLOR_TAN IM_COL32(210,180,140,255)
#define KNOWNIMGUICOLOR_TEAL IM_COL32(0,128,128,255)
#define KNOWNIMGUICOLOR_THISTLE IM_COL32(216,191,216,255)
#define KNOWNIMGUICOLOR_TOMATO IM_COL32(255,99,71,255)
#define KNOWNIMGUICOLOR_TURQUOISE IM_COL32(64,224,208,255)
#define KNOWNIMGUICOLOR_VIOLET IM_COL32(238,130,238,255)
#define KNOWNIMGUICOLOR_WHEAT IM_COL32(245,222,179,255)
#define KNOWNIMGUICOLOR_WHITE IM_COL32(255,255,255,255)
#define KNOWNIMGUICOLOR_WHITESMOKE IM_COL32(245,245,245,255)
#define KNOWNIMGUICOLOR_YELLOW IM_COL32(255,255,0,255)
#define KNOWNIMGUICOLOR_YELLOWGREEN IM_COL32(154,205,50,255)
#endif // NO_IMGUIKNOWNCOLOR_DEFINITIONS

#endif //IMGUIHELPER_H_

