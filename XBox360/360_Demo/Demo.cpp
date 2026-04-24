#include <xtl.h>
#include <xboxmath.h>
#include <iostream>
#include <xboxmath.h>
#include <AtgApp.h>
#include <AtgFont.h>
#include <AtgMesh.h>
#include <AtgInput.cpp>
#include <AtgResource.h>
#include <AtgUtil.h>

const char* g_strVertexShaderProgram = 
" float4x4 matWVP : register(c0);              "  
"                                              "  
" struct VS_IN                                 "  
" {                                            " 
"     float4 ObjPos   : POSITION;              "  // Object space position 
"     float4 Color    : COLOR;                 "  // Vertex color                 
" };                                           " 
"                                              " 
" struct VS_OUT                                " 
" {                                            " 
"     float4 ProjPos  : POSITION;              "  // Projected space position 
"     float4 Color    : COLOR;                 "  
" };                                           "  
"                                              "  
" VS_OUT main( VS_IN In )                      "  
" {                                            "  
"     VS_OUT Out;                              "  
"     Out.ProjPos = mul( matWVP, In.ObjPos );  "  // Transform vertex into
"     Out.Color = In.Color;                    "  // Projected space and 
"     return Out;                              "  // Transfer color
" } " ;  

//-------------------------------------------------------------------------------------
// Pixel shader
//-------------------------------------------------------------------------------------
const char* g_strPixelShaderProgram = 
" struct PS_IN                                 "
" {                                            "
"     float4 Color : COLOR;                    "  // Interpolated color from                      
" };                                           "  // the vertex shader
"                                              "  
" float4 main( PS_IN In ) : COLOR              "  
" {                                            "  
"     return In.Color;                         "  // Output color
" } " ;

struct COLORVERTEX
{
    float       Position[3];
    DWORD       Color;
};

//-------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------
D3DDevice*             g_pd3dDevice;    // Our rendering device
D3DVertexBuffer*       g_pVB;           // Buffer to hold vertices
D3DVertexDeclaration*  g_pVertexDecl;   // Vertex format decl
D3DVertexShader*       g_pVertexShader; // Vertex Shader
D3DPixelShader*        g_pPixelShader;  // Pixel Shader

XMMATRIX g_matWorld;
XMMATRIX g_matProj;
XMMATRIX g_matView;

BOOL g_bWidescreen = TRUE;

class Demo_360 : public ATG::Application
{
    // Valid app states
    enum APPSTATE
    {
        APPSTATE_CONTROLTEST=0,
        APPSTATE_VIBRATIONTEST,
        APPSTATE_DEADZONECALIBRATION,
        APPSTATE_BUTTONQUANTIZATION,
        APPSTATE_MEMORYUNITDETECTION,
        APPSTATE_MAX,
    };

    // General application members
    APPSTATE m_AppState;         // State of the app
    ATG::Timer m_Timer;            // Timer for the app

    // Active gamepad
    ATG::GAMEPAD* m_pGamepad;

    // Vibration motor values
    FLOAT m_fLeftMotorSpeed;
    FLOAT m_fRightMotorSpeed;

    // Deadzone calibration page
    FLOAT m_fDeadZone;

    // Control quantization page
    BYTE* m_pQuantizedThumbStickValues;
    BYTE* m_pQuantizedButtonValues;

    LPDIRECT3DVERTEXDECLARATION9 m_pScreenspaceDecl;
    LPDIRECT3DVERTEXDECLARATION9 m_pScreenspaceTexcoordDecl;

    LPDIRECT3DVERTEXSHADER9 m_pScreenspaceVS;
    LPDIRECT3DVERTEXSHADER9 m_pScreenspaceTexcoordVS;
    LPDIRECT3DPIXELSHADER9 m_pConstantColorPS;
    LPDIRECT3DPIXELSHADER9 m_pTexturePS;

    // Internal members
    VOID            ShowTexture( LPDIRECT3DTEXTURE9 pTexture );
    HRESULT         DrawBox( LONG x1, LONG y1, LONG x2, LONG y2,
                             const XMFLOAT4& vFillColor,
                             const XMFLOAT4& vwOutlineColor );

    HRESULT         RenderControlTestPage();
    HRESULT         RenderControlTestPageUI();

    HRESULT         RenderVibrationTestPage();
    HRESULT         RenderVibrationTestPageUI();

    HRESULT         RenderDeadZoneCalibrationPage();
    HRESULT         RenderDeadZoneCalibrationPageUI();

    HRESULT         RenderButtonQuantizationPage();
    HRESULT         RenderButtonQuantizationPageUI();

public:
    virtual HRESULT Initialize();
	virtual HRESULT InitD3D();
	virtual HRESULT InitScene();
	virtual HRESULT InitApp();
    virtual HRESULT Update();
    virtual HRESULT Render();
};

void _cdecl main()
{
	Demo_360 app;

	if( FAILED( app.InitD3D() ) )
        return;

	// Initialize the vertex buffer
    if( FAILED( app.InitScene() ) )
        return;

	for (;;)
	{
		app.Update();
		app.Render();
	}
}

HRESULT Demo_360::Initialize()
{
	return S_OK;
}

HRESULT Demo_360::InitD3D()
{
	Direct3D* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!pD3D)
		return E_FAIL;

	 D3DPRESENT_PARAMETERS d3dpp; 
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    XVIDEO_MODE VideoMode;
    XGetVideoMode( &VideoMode );
    g_bWidescreen = VideoMode.fIsWideScreen;
    d3dpp.BackBufferWidth        = min( VideoMode.dwDisplayWidth, 1280 );
    d3dpp.BackBufferHeight       = min( VideoMode.dwDisplayHeight, 720 );
    d3dpp.BackBufferFormat       = D3DFMT_X8R8G8B8;
    d3dpp.BackBufferCount        = 1;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_ONE;

	    // Create the Direct3D device.
    if( FAILED( pD3D->CreateDevice( 0, D3DDEVTYPE_HAL, NULL,
                                    D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                    &d3dpp, &g_pd3dDevice ) ) )
        return E_FAIL;

    return S_OK;
}

HRESULT Demo_360::InitScene()
{
	ID3DXBuffer* pVertexShaderCode;
	ID3DXBuffer* pVertexErrorMsg;
	HRESULT hr = D3DXCompileShader(  g_strVertexShaderProgram,
									 (UINT)strlen( g_strVertexShaderProgram ),
                                    NULL, 
                                    NULL, 
                                    "main", 
                                    "vs_2_0", 
                                    0, 
                                    &pVertexShaderCode, 
                                    &pVertexErrorMsg, 
                                    NULL );
	 if( FAILED(hr) )
    {
        if( pVertexErrorMsg )
            OutputDebugString( (char*)pVertexErrorMsg->GetBufferPointer() );
        return E_FAIL;
    }    

    // Create vertex shader.
    g_pd3dDevice->CreateVertexShader( (DWORD*)pVertexShaderCode->GetBufferPointer(), 
                                      &g_pVertexShader );

    // Compile pixel shader.
    ID3DXBuffer* pPixelShaderCode;
    ID3DXBuffer* pPixelErrorMsg;
    hr = D3DXCompileShader( g_strPixelShaderProgram, 
                            (UINT)strlen( g_strPixelShaderProgram ),
                            NULL, 
                            NULL, 
                            "main", 
                            "ps_2_0", 
                            0, 
                            &pPixelShaderCode, 
                            &pPixelErrorMsg,
                            NULL );
    if( FAILED(hr) )
    {
        if( pPixelErrorMsg )
            OutputDebugString( (char*)pPixelErrorMsg->GetBufferPointer() );
        return E_FAIL;
    }

    // Create pixel shader.
    g_pd3dDevice->CreatePixelShader( (DWORD*)pPixelShaderCode->GetBufferPointer(), 
                                     &g_pPixelShader );
	
	D3DVERTEXELEMENT9 VertexElements[3] =
    {
        { 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
        D3DDECL_END()
    };
    g_pd3dDevice->CreateVertexDeclaration( VertexElements, &g_pVertexDecl );

    // Create the vertex buffer. Here we are allocating enough memory
    // (from the default pool) to hold all our 3 custom vertices. 
    if( FAILED( g_pd3dDevice->CreateVertexBuffer( 3*sizeof(COLORVERTEX),
                                                  D3DUSAGE_WRITEONLY, 
                                                  NULL,
                                                  D3DPOOL_MANAGED, 
                                                  &g_pVB, 
                                                  NULL ) ) )
        return E_FAIL;

    // Now we fill the vertex buffer. To do this, we need to Lock() the VB to
    // gain access to the vertices. This mechanism is required because the
    // vertex buffer may still be in use by the GPU. This can happen if the
    // CPU gets ahead of the GPU. The GPU could still be rendering the previous
    // frame.
    COLORVERTEX g_Vertices[] =
    {
        {  0.0f, -1.1547f, 0.0f, 0xffff0000 }, // x, y, z, color
        { -1.0f,  0.5777f, 0.0f, 0xff00ff00 },
        {  1.0f,  0.5777f, 0.0f, 0xffffff00 },
    };

    COLORVERTEX* pVertices;
    if( FAILED( g_pVB->Lock( 0, 0, (void**)&pVertices, 0 ) ) )
        return E_FAIL;
    memcpy( pVertices, g_Vertices, 3*sizeof(COLORVERTEX) );
    g_pVB->Unlock();

	g_matWorld = XMMatrixIdentity();

	// Projection Matrix
	FLOAT fAspect = ( g_bWidescreen ) ? (16.0f / 9.0f) : (4.0f / 3.0f);
	g_matProj = XMMatrixPerspectiveFovLH(XM_PIDIV4, fAspect, 1.0f, 200.0f);

	// Initialize the view matrix
    XMVECTOR vEyePt    = { 0.0f, 0.0f,-7.0f, 0.0f };
    XMVECTOR vLookatPt = { 0.0f, 0.0f, 0.0f, 0.0f };
    XMVECTOR vUp       = { 0.0f, 1.0f, 0.0f, 0.0f };
    g_matView = XMMatrixLookAtLH( vEyePt, vLookatPt, vUp );

    return S_OK;
}

HRESULT Demo_360::InitApp()
{
    m_AppState = APPSTATE_CONTROLTEST;
    m_fDeadZone = 0.24f;  // Set default deadzone to 24%
    m_fLeftMotorSpeed = 0.0f;
    m_fRightMotorSpeed = 0.0f;

    // Quantized control values
    m_pQuantizedThumbStickValues = new BYTE[256];
    ZeroMemory( m_pQuantizedThumbStickValues, 256 );

    m_pQuantizedButtonValues = new BYTE[256];
    ZeroMemory( m_pQuantizedButtonValues, 256 );

	return S_OK;
}

HRESULT Demo_360::Update()
{
	// Get the current gamepad status
	m_pGamepad = ATG::Input::GetMergedInput();

	if( m_pGamepad->wPressedButtons & XINPUT_GAMEPAD_DPAD_UP )
	{
            XMMATRIX Translation = XMMatrixTranslation(0, 1, 0);
			g_matWorld = g_matWorld * Translation;
	}

	if( m_pGamepad->wPressedButtons & XINPUT_GAMEPAD_DPAD_DOWN )
	{
            XMMATRIX Translation = XMMatrixTranslation(0, -1, 0);
			g_matWorld = g_matWorld * Translation;
	}

	if( m_pGamepad->wPressedButtons & XINPUT_GAMEPAD_DPAD_LEFT )
	{
            XMMATRIX Translation = XMMatrixTranslation(-1, 0, 0);
			g_matWorld = g_matWorld * Translation;
	}

	if( m_pGamepad->wPressedButtons & XINPUT_GAMEPAD_DPAD_RIGHT )
	{
            XMMATRIX Translation = XMMatrixTranslation(1, 0, 0);
			g_matWorld = g_matWorld * Translation;
	}


	return S_OK;
}

HRESULT Demo_360::Render()
{
	g_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, D3DCOLOR_XRGB(0,0,255), 1.0f, 0L);

	g_pd3dDevice->SetVertexDeclaration( g_pVertexDecl );
    g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(COLORVERTEX) );
    g_pd3dDevice->SetVertexShader( g_pVertexShader );
    g_pd3dDevice->SetPixelShader( g_pPixelShader );
    
	XMMATRIX matWVP = g_matWorld * g_matView * g_matProj;
	g_pd3dDevice->SetVertexShaderConstantF(0, (FLOAT*)&matWVP, 4);

	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 1);
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

	return S_OK;
}

