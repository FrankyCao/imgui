// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

#include "imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <string>
#include <sstream>
#include <fstream>
#include "ImGuiFileDialog.h"
#include <SDL.h>


#define FONT_ICON_BUFFER_NAME_IGFD IGFD_compressed_data_base85
#define FONT_ICON_BUFFER_SIZE_IGFD 0xc21

#define ICON_MIN_IGFD 0xf002
#define ICON_MAX_IGFD 0xf1c9

#define ICON_IGFD_ADD u8"\uf067"
#define ICON_IGFD_BOOKMARK u8"\uf02e"
#define ICON_IGFD_CANCEL u8"\uf00d"
#define ICON_IGFD_DRIVES u8"\uf0a0"
#define ICON_IGFD_EDIT u8"\uf040"
#define ICON_IGFD_FILE u8"\uf15b"
#define ICON_IGFD_FILE_PIC u8"\uf1c5"
#define ICON_IGFD_FOLDER u8"\uf07b"
#define ICON_IGFD_FOLDER_OPEN u8"\uf07c"
#define ICON_IGFD_LINK u8"\uf1c9"
#define ICON_IGFD_OK u8"\uf00c"
#define ICON_IGFD_REFRESH u8"\uf021"
#define ICON_IGFD_REMOVE u8"\uf068"
#define ICON_IGFD_RESET u8"\uf064"
#define ICON_IGFD_SAVE u8"\uf0c7"
#define ICON_IGFD_SEARCH u8"\uf002"

static const char FONT_ICON_BUFFER_NAME_IGFD[3105+1] =
    "7])#######qgGmo'/###V),##+Sl##Q6>##w#S+Hh=?<a7*&T&d.7m/oJ[^IflZg#BfG<-iNE/1-2JuBw0'B)i,>>#'tEn/<_[FHkp#L#,)m<-:qEn/@d@UCGD7s$_gG<-]rK8/XU#[A"
    ">7X*M^iEuLQaX1DIMr62DXe(#=eR%#_AFmBFF1J5h@6gLYwG`-77LkOETt?0(MiSAq@ClLS[bfL)YZ##E)1w--Aa+MNq;?#-D^w'0bR5'Cv9N(f$IP/371^#IhOSMoH<mL6kSG2mEexF"
    "TP'##NjToIm3.AF4@;=-1`/,M)F5gL':#gLIlJGMIfG<-IT*COI=-##.<;qMTl:$#L7cwL#3#&#W(^w5i*l.q3;02qtKJ5q'>b9qx:`?q*R[SqOim4vII3L,J-eL,o[njJ@Ro?93VtA#"
    "n;4L#?C(DNgJG&#D;B;-KEII-O&Ys1,AP##0Mc##4Yu##fRXgLC;E$#@(V$#jDA]%f,);?o[3YckaZfCj>)Mp64YS76`JYYZGUSItO,AtgC_Y#>v&##je+gL)SL*57*vM($i?X-,BG`a"
    "*HWf#Ybf;-?G^##$VG13%&>uuYg8e$UNc##D####,03/MbPMG)@f)T/^b%T%S2`^#J:8Y.pV*i(T=)?#h-[guT#9iu&](?#A]wG;[Dm]uB*07QK(]qFV=fV$H[`V$#kUK#$8^fLmw@8%"
    "P92^uJ98=/+@u2OFC.o`5BOojOps+/q=110[YEt$bcx5#kN:tLmb]s$wkH>#iE(E#VA@r%Mep$-#b?1,G2J1,mQOhuvne.Mv?75/Huiw'6`?$=VC]&,EH*7M:9s%,:7QVQM]X-?^10ip"
    "&ExrQF7$##lnr?#&r&t%NEE/2XB+a4?c=?/^0Xp%kH$IM$?YCjwpRX:CNNjL<b7:.rH75/1uO]unpchLY.s%,R=q9V%M,)#%)###Tx^##x@PS.kN%##AFDZ#5D-W.-kr.0oUFb35IL,3"
    "%A2*/RtC.3j85L#gJ))3rx3I):2Cv-FX(9/vBo8%=[%?5BKc8/t=r$#<MfD*.gB.*Q&WS7#r&ipf9b^EhD]:/%A@`aU5b'O]I03N2cUN0ebYF':?AE+OaUq).]tfd]s0$d0C9E#enTtQ"
    "&(oqL^)sB#`:5Yu9A;TFN%MeMBhZd3J0(6:8mc0CdpC,)#5>X(3^*rMo&Y9%)[[c;QIt<1q3n0#MI`QjeUB,MCG#&#n#]I*/=_hLfM]s$Cr&t%#M,W-d9'hlM2'J35i4f)_Y_Z-Mx;=."
    "Z&f.:a[xw'2q'Y$G38$5Zs<7.;G(<-$87V?M42X-n+[w'1q-[Bv(ofL2@R2L/%dDE=?CG)bhho.&1(a4D/NF3;G`=.Su$s$]WD.3jY5lLBMuM(Hnr?#FTFo26N.)*,/_Yox3prHH]G>u"
    "?#Ke$+tG;%M)H3uH@ta$/$bku/1.E4:po2/Z5wGE^e*:*Mgj8&=B]'/-=h1B-n4GVaVQu.gm^6X,&Gj/Run+M,jd##kJ,/1=-U,2QNv)42.,Q'iUKF*wCXI)+f1B4./.&4maJX-'$fF4"
    "uX@8%5Y,>#r:N&l,=GNGA5AZ$XA`0(Q^(k'(GB.WKx+M/mi5###%D1Mh;BE+O.1w,p8QSV`E3$%sMwH)/t6iLF.'DX[G&8Remo7/,w/RNRPUV$)8[0#G,>>#d=OZ-$_Aj0^ll%0uAOZ6"
    "m('J3*`4-6Rq@.*G7K,3L1O058b4-5mS4'5841'5%J/GV/1:B#'at%$q^DIM=X0DMg*^fL6)0/Ldbb(NRdimM-a4o7qPa>$cMDX:W<=&5t^Dv$)2mJ)lb+Ze`eHQ%:oSfLiMK/L@i6o7"
    ":fKb@'x_5/Q1wPMIR0cMmbR`aRLb>-w..e-R3n0#bsn`$bA%%#N,>>#MhSM'Z`qdm?D,c4A]DD3mMWB#,5Rv$g&?a3?9wGMeANv>s#V&1iJ&%bE2ZA#-.^g1j;R)4443%b7RU`u`Kk(N"
    "HbeV@gJS'-BWcP3m?+#-VaDuL:`WY5kEaau^=lJ(_24J-WvW+.VxIfLXqd##Q3=&#d72mLvG(u$+dfF4<BPF%MPEb3:]Me.d(4I)(P;e.)_K#$^n;#MjXGp%jDJeM20P.)'9_-2[x):8"
    ",VjfLQJa0V*#4o7dD24'^bO-)GX3/U@@%P'9/8b*;XmGA9Gw;8i=I`(sZhM'8A]?cof-6M>Awx-tS<GMRoS+MWWB@#WG9J'P@)?uNUYI8#-EE#[Yw_#;R,<8+b?:3=t$gLFf]0#G?O&#"
    "DoA*#_>QJ(PGpkL'(MB#/T01YP6;hLeb6lL7$(f):3ou-KHaJM[TXD#'1;p72$[p.Z9OA#RcB##Oi./LlA)ZuTBqn'B]7%b/WM<LrFcS7XOtILd9b<UxX'^#)J/Dt]il3++^tpAu_L%,"
    "w$[gu5[-['#**&+*wwGXO4h=#%H3'50S6##l9f/)m`):)t1@k=?\?#]u3i8-#%/5##)]$s$+JNh#jl###cU^F*Vs'Y$Lov[-0<a?$Hni?#+?2?up1m%.*%%-NPB`>$agNe'Qk[X'ep.'5"
    "8=B_A+L1_A'fp`Nae&%#aKb&#W>gkLZ/(p$V2Cv-_uv20tt?X-BL75/#(KU)N0;hLr75c4br9s-;va.3exLG`^U;4F9D+tqKKGSIULS:d=vRduSxXCuOq$0ufu'L#>Y[OVk1k3G(kZoA"
    "O9iQN'h5',b_mL,v?qr&4uG##%J/GV5J?`a'=@@M#w-tL0+xRV6,A48_fa-Zups+;(=rhZ;ktD#c9OA#axJ+*?7%s$DXI5/CKU:%eQ+,2=C587Rg;E4Z3f.*?ulW-tYqw0`EmS/si+C#"
    "[<;S&mInS%FAov#Fu[E+*L'O'GWuN'+)U40`a5k'sA;)*RIRF%Tw/Z--1=e?5;1X:;vPk&CdNjL*(KJ1/,TV-G(^S*v14gL#8,,M*YPgLaII@b+s[&#n+d3#1jk$#';P>#,Gc>#0Su>#"
    "4`1?#8lC?#<xU?#@.i?#D:%@#i'LVCn)fQD[.C(%ea@uBoF/ZGrDFVCjZ/NB0)61Fg&cF$v:7FHFDRb3bW<2Be(&ZG17O(Ie4;hFwf1eGEmqA4sS4VCoC%eG1PM*HsH%'I]MBnD+'],M"
    "w)n,Ga8q`EgKJ.#wZ/x=v`#/#";


// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static bool canValidateDialog = false;

inline void InfosPane(std::string vFilter, igfd::UserDatas vUserDatas, bool *vCantContinue) // if vCantContinue is false, the user cant validate the dialog
{
	ImGui::TextColored(ImVec4(0, 1, 1, 1), "Infos Pane");
	
	ImGui::Text("Selected Filter : %s", vFilter.c_str());

	const char* userDatas = (const char*)vUserDatas;
	if (userDatas)
        ImGui::Text("User Datas : %s", userDatas);

	ImGui::Checkbox("if not checked you cant validate the dialog", &canValidateDialog);

	if (vCantContinue)
	    *vCantContinue = canValidateDialog;
}

inline bool RadioButtonLabeled(const char* label, bool active, bool disabled)
{
	using namespace ImGui;

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	float w = CalcItemWidth();
	if (w == window->ItemWidthDefault)	w = 0.0f; // no push item width
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, nullptr, true);
	ImVec2 bb_size = ImVec2(style.FramePadding.x * 2 - 1, style.FramePadding.y * 2 - 1) + label_size;
	bb_size.x = ImMax(w, bb_size.x);

	const ImRect check_bb(
		window->DC.CursorPos,
		window->DC.CursorPos + bb_size);
	ItemSize(check_bb, style.FramePadding.y);

	if (!ItemAdd(check_bb, id))
		return false;

	// check
	bool pressed = false;
	if (!disabled)
	{
		bool hovered, held;
		pressed = ButtonBehavior(check_bb, id, &hovered, &held);

		window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), style.FrameRounding);
		if (active)
		{
			const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
			window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, col, style.FrameRounding);
		}
	}

	// circle shadow + bg
	if (style.FrameBorderSize > 0.0f)
	{
		window->DrawList->AddRect(check_bb.Min + ImVec2(1, 1), check_bb.Max, GetColorU32(ImGuiCol_BorderShadow), style.FrameRounding);
		window->DrawList->AddRect(check_bb.Min, check_bb.Max, GetColorU32(ImGuiCol_Border), style.FrameRounding);
	}

	if (label_size.x > 0.0f)
	{
		RenderText(check_bb.GetCenter() - label_size * 0.5f, label);
	}

	return pressed;
}

int main(int, char**)
{
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#ifdef __APPLE__
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
    bool err = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    bool err = false;
    glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    bool err = false;
    glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)SDL_GL_GetProcAddress(name); });
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = "file_dialog.ini";
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.FontAllowUserScaling = true; // zoom wiht ctrl + mouse wheel 

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

	// load icon font file (CustomFont.cpp)
	ImGui::GetIO().Fonts->AddFontDefault();
	static const ImWchar icons_ranges[] = { ICON_MIN_IGFD, ICON_MAX_IGFD, 0 };
	ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
	ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_IGFD, 15.0f, &icons_config, icons_ranges);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".cpp", ImVec4(1.0f, 1.0f, 0.0f, 0.9f));
	igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".h", ImVec4(0.0f, 1.0f, 0.0f, 0.9f));
	igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".hpp", ImVec4(0.0f, 0.0f, 1.0f, 0.9f));
	igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".md", ImVec4(1.0f, 0.0f, 1.0f, 0.9f));
	igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".png", ImVec4(0.0f, 1.0f, 1.0f, 0.9f), ICON_IGFD_FILE_PIC); // add an icon for the filter type
	igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".gif", ImVec4(0.0f, 1.0f, 0.5f, 0.9f), "[GIF]"); // add an text for a filter type

#ifdef USE_BOOKMARK
	// load bookmarks
	std::ifstream docFile("bookmarks.conf", std::ios::in);
	if (docFile.is_open())
	{
		std::stringstream strStream;
		strStream << docFile.rdbuf();//read the file
		igfd::ImGuiFileDialog::Instance()->DeserializeBookmarks(strStream.str());
		docFile.close();
	}
#endif

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);
			ImGui::Separator();

			ImGui::Text("imGuiFileDialog Demo %s : ", IMGUIFILEDIALOG_VERSION);
			ImGui::Indent();
			{
#ifdef USE_EXPLORATION_BY_KEYS
				static float flashingAttenuationInSeconds = 1.0f;
				if (ImGui::Button("R##resetflashlifetime"))
				{
					flashingAttenuationInSeconds = 1.0f;
					igfd::ImGuiFileDialog::Instance()->SetFlashingAttenuationInSeconds(flashingAttenuationInSeconds);
				}
				ImGui::SameLine();
				ImGui::PushItemWidth(200);
				if (ImGui::SliderFloat("Flash lifetime (s)", &flashingAttenuationInSeconds, 0.01f, 5.0f))
					igfd::ImGuiFileDialog::Instance()->SetFlashingAttenuationInSeconds(flashingAttenuationInSeconds);
				ImGui::PopItemWidth();
#endif
				static bool _UseWindowContraints = true;
				ImGui::Separator();
				ImGui::Checkbox("Use file dialog constraint", &_UseWindowContraints);
				ImGui::Text("Constraints is used here for define min/ax fiel dialog size");
				ImGui::Separator();
				static bool standardDialogMode = true;
				ImGui::Text("Open Mode : ");
				ImGui::SameLine();
				if (RadioButtonLabeled("Standard", standardDialogMode, false)) standardDialogMode = true;
				ImGui::SameLine();
				if (RadioButtonLabeled("Modal", !standardDialogMode, false)) standardDialogMode = false;

				if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog"))
				{
					const char *filters = ".*,.cpp,.h,.hpp";
					if (standardDialogMode)
						igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", 
							ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".");
					else
						igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey", 
							ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".");
				}
				if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with collections of filters"))
				{
					const char *filters = "Source files (*.cpp *.h *.hpp){.cpp,.h,.hpp},Image files (*.png *.gif *.jpg *.jpeg){.png,.gif,.jpg,.jpeg},.md";
					if (standardDialogMode)
						igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey",
							ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".");
					else
						igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey",
							ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".");
				}
				if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with selection of 5 items"))
				{
					const char *filters = ".*,.cpp,.h,.hpp";
					if (standardDialogMode)
						igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey",
							ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", 5);
					else
						igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey",
							ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", 5);
				}
				if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with infinite selection"))
				{
					const char *filters = ".*,.cpp,.h,.hpp";
					if (standardDialogMode)
						igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey",
							ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", 0);
					else
						igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey",
							ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", 0);
				}
				if (ImGui::Button(ICON_IGFD_SAVE " Save File Dialog with a custom pane"))
				{
					const char *filters = "C++ File (*.cpp){.cpp}";
					if (standardDialogMode)
						igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey",
							ICON_IGFD_SAVE " Choose a File", filters,
							".", "", std::bind(&InfosPane, std::placeholders::_1, std::placeholders::_2, 
							std::placeholders::_3), 350, 1, igfd::UserDatas("SaveFile"));
					else
						igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey",
							ICON_IGFD_SAVE " Choose a File", filters,
							".", "", std::bind(&InfosPane, std::placeholders::_1, std::placeholders::_2,
								std::placeholders::_3), 350, 1, igfd::UserDatas("SaveFile"));
				}
                if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open Directory Dialog"))
                {
					// set filters to 0 for open directory chooser
					if (standardDialogMode)
						igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseDirDlgKey",
						ICON_IGFD_FOLDER_OPEN " Choose a Directory", 0, ".");
					else
						igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseDirDlgKey",
							ICON_IGFD_FOLDER_OPEN " Choose a Directory", 0, ".");
                }
                if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open Directory Dialog with selection of 5 items"))
                {
					// set filters to 0 for open directory chooser
					if (standardDialogMode)
						igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseDirDlgKey",
							ICON_IGFD_FOLDER_OPEN " Choose a Directory", 0, ".", 5);
					else
						igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseDirDlgKey",
							ICON_IGFD_FOLDER_OPEN " Choose a Directory", 0, ".", 5);
                }

                ImVec2 minSize = ImVec2(0, 0);
				ImVec2 maxSize = ImVec2(FLT_MAX, FLT_MAX);

				if (_UseWindowContraints)
				{
					maxSize = ImVec2((float)io.DisplaySize.x, (float)io.DisplaySize.y);
					minSize = maxSize * 0.5f;
				}

				// you can define your flags and min/max window size (theses three settings ae defined by default :
				// flags => ImGuiWindowFlags_NoCollapse
				// minSize => 0,0
				// maxSize => FLT_MAX, FLT_MAX (defined is float.h)

				if (igfd::ImGuiFileDialog::Instance()->FileDialog("ChooseFileDlgKey",
				        ImGuiWindowFlags_NoCollapse, minSize, maxSize))
				{
					if (igfd::ImGuiFileDialog::Instance()->IsOk)
					{
						std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
						std::string filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath();
						std::string filter = igfd::ImGuiFileDialog::Instance()->GetCurrentFilter();
						// here convert from string because a string was passed as a userDatas, but it can be what you want
                        std::string userDatas;
						if (igfd::ImGuiFileDialog::Instance()->GetUserDatas())
                            userDatas = std::string((const char*)igfd::ImGuiFileDialog::Instance()->GetUserDatas());
						auto selection = igfd::ImGuiFileDialog::Instance()->GetSelection(); // multiselection

						// action
					}
					igfd::ImGuiFileDialog::Instance()->CloseDialog("ChooseFileDlgKey");
				}

				if (igfd::ImGuiFileDialog::Instance()->FileDialog("ChooseDirDlgKey",
				        ImGuiWindowFlags_NoCollapse, minSize, maxSize))
                {
                    if (igfd::ImGuiFileDialog::Instance()->IsOk)
                    {
                        std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
                        std::string filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath();
                        std::string filter = igfd::ImGuiFileDialog::Instance()->GetCurrentFilter();
                        // here convert from string because a string was passed as a userDatas, but it can be what you want
                        std::string userDatas;
                        if (igfd::ImGuiFileDialog::Instance()->GetUserDatas())
                            userDatas = std::string((const char*)igfd::ImGuiFileDialog::Instance()->GetUserDatas());
                        auto selection = igfd::ImGuiFileDialog::Instance()->GetSelection(); // multiselection

                        // action
                    }
                    igfd::ImGuiFileDialog::Instance()->CloseDialog("ChooseDirDlgKey");
                }
			}
			ImGui::Unindent();

			ImGui::Separator();
			ImGui::Text("Window mode :");
			ImGui::Separator();

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

#ifdef USE_BOOKMARK
	// save bookmarks
	std::ofstream configFileWriter("bookmarks.conf", std::ios::out);
	if (!configFileWriter.bad())
	{
		configFileWriter << igfd::ImGuiFileDialog::Instance()->SerializeBookmarks();
		configFileWriter.close();
	}
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
