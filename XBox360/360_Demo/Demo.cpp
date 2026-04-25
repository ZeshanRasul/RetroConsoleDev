#include <xtl.h>
#include <xboxmath.h>

#include <iostream>

#include <xgraphics.h>
#include <AtgApp.h>
#include <AtgFont.h>

#include <AtgMesh.h>

#include <AtgHelp.h>
#include <AtgInput.h>

#include <AtgResource.h>

#include <AtgUtil.h>

#include <d3d9.h>


//--------------------------------------------------------------------------------------
// Vertex shader
//--------------------------------------------------------------------------------------
const CHAR*         g_strVertexShaderProgram =
    "                                              "
    "                                              "
    " float4x4 matWVP : register(c0);              "
    "												"
	" float4 gLightVecW : register(c4);    			"
	" float4 gDiffuseLight : register(c5); 			"
	"float4x4 g_InvWorld : register(c6);											"
	" struct VS_IN                                 "
    " {                                            "
    "     float3 ObjPos : POSITION0;                "  
	"	  float3 Norm   : NORMAL;	"
    "     float2 UV     : TEXCOORD0;                  "  
    " };                                           "
    "                                              "
    " struct VS_OUT                                "
    " {                                            "
    "     float4 ProjPos  : POSITION0;              "  
	"	  float2 UV         : TEXCOORD0;"
    " };                                           "
    "                                              "
    " VS_OUT main( VS_IN In )                      "
    " {                                            "
    "     VS_OUT Out;                              "
	"     Out.ProjPos = mul(float4(In.ObjPos, 1.0f), matWVP );  " 
	"     float3 normalW = mul(float4(In.Norm, 0.0f), g_InvWorld).xyz;       "
	"	  normalW = normalize(normalW); "
	"     float s = max(dot(gLightVecW, normalW), 0.0f);"
	"	  Out.UV = In.UV;	"
    "     return Out;                              "  
    " }                                            ";


//-------------------------------------------------------------------------------------
// Pixel shader
//-------------------------------------------------------------------------------------
const CHAR*         g_strPixelShaderProgram =
    " sampler2D ColorTexture : register(s0);       "
	"         "
	"         "
    " struct PS_IN                                 "
    " {                                            "
   "     float4 ProjPos  : POSITION0;              "  
	"      float2 UV          : TEXCOORD0;"
    " };                                           "  
    "                                              "
    " float4 main( PS_IN In ) : COLOR              "
    " {                                            "
	"     float3 texColor = tex2D(ColorTexture, In.UV).rgb;"
	"     float3 diffuse = texColor; "
    "     return float4(diffuse, 1.0f);    "  
    " }                                            ";

//-------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------
D3DDevice*             g_pd3dDevice;    // Our rendering device


XMMATRIX g_matWorld;
XMMATRIX g_matWorld2;
XMMATRIX g_matProj;
XMMATRIX g_matView;
XMMATRIX g_InvWorld;
XMMATRIX g_InvWorld2;

BOOL g_bWidescreen = TRUE;
float dt = 1.0f / 60.0f;
//--------------------------------------------------------------------------------------
// Globals variables and definitions
//--------------------------------------------------------------------------------------
// Structure to hold vertex data.
struct BOXVERTEX
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
	XMFLOAT2 UV;
};

//Unit Box
BOXVERTEX g_BoxVertices[4*6] =
{
    // Front
    { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, -1.0f ), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(  1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, -1.0f ), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(  1.0f,-1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, -1.0f ), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3( -1.0f,-1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, -1.0f ), XMFLOAT2(0.0f, 1.0f) },

    // Back
    { XMFLOAT3( -1.0f, 1.0f,  1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f ), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(  1.0f, 1.0f,  1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f ), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(  1.0f,-1.0f,  1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f ), XMFLOAT2(0.0f, 1.0f) },
    { XMFLOAT3( -1.0f,-1.0f,  1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f ), XMFLOAT2(1.0f, 1.0f) },

    // Left
    { XMFLOAT3( -1.0f,-1.0f,  1.0f ), XMFLOAT3(-1.0f, 0.0f, 0.0f ), XMFLOAT2(0.0f, 1.0f) },
    { XMFLOAT3( -1.0f, 1.0f,  1.0f ), XMFLOAT3(-1.0f, 0.0f, 0.0f ), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3(-1.0f, 0.0f, 0.0f ), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3( -1.0f,-1.0f, -1.0f ), XMFLOAT3(-1.0f, 0.0f, 0.0f ), XMFLOAT2(1.0f, 1.0f) },

    // Right
    { XMFLOAT3( 1.0f,-1.0f,  1.0f ), XMFLOAT3(1.0f, 0.0f, 0.0f ), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3( 1.0f, 1.0f,  1.0f ), XMFLOAT3(1.0f, 0.0f, 0.0f ), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT3(1.0f, 0.0f, 0.0f ), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3( 1.0f,-1.0f, -1.0f ), XMFLOAT3(1.0f, 0.0f, 0.0f ), XMFLOAT2(0.0f, 1.0f) },

    // Bottom
    { XMFLOAT3( -1.0f,-1.0f,  1.0f ), XMFLOAT3(0.0f,-1.0f, 0.0f ), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(  1.0f,-1.0f,  1.0f ), XMFLOAT3(0.0f,-1.0f, 0.0f ), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(  1.0f,-1.0f, -1.0f ), XMFLOAT3(0.0f,-1.0f, 0.0f ), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3( -1.0f,-1.0f, -1.0f ), XMFLOAT3(0.0f,-1.0f, 0.0f ), XMFLOAT2(0.0f, 1.0f) },

    // Top
    { XMFLOAT3( -1.0f, 1.0f,  1.0f ), XMFLOAT3(0.0f, 1.0f, 0.0f ), XMFLOAT2(0.0f, 1.0f) },
    { XMFLOAT3(  1.0f, 1.0f,  1.0f ), XMFLOAT3(0.0f, 1.0f, 0.0f ), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(  1.0f, 1.0f, -1.0f ), XMFLOAT3(0.0f, 1.0f, 0.0f ), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3(0.0f, 1.0f, 0.0f ), XMFLOAT2(0.0f, 0.0f) },
};

class Demo_360 : public ATG::Application
{
	ATG::Timer m_Timer;    // Timer
    ATG::Font m_Font;     // Font for drawing text
    ATG::Help m_Help;
    BOOL m_bDrawHelp;
    BOOL m_bFloatDepth;
    BOOL m_bWireframe;

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
            // Timer for the app

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

	void BuildViewMatrix();

	// Vertex buffers
	LPDIRECT3DVERTEXBUFFER9 m_pInnerBoxVB;
	LPDIRECT3DVERTEXBUFFER9 m_pOuterBoxVB;

	// Shaders
	LPDIRECT3DVERTEXSHADER9 m_pBoxVS;
	LPDIRECT3DPIXELSHADER9 m_pBoxPS;

	IDirect3DTexture9* m_Texture;
	IDirect3DVertexDeclaration9* m_VertexDecl;
	IDirect3DVertexShader9* m_VertexShader;
	IDirect3DPixelShader9* m_PixelShader;
	IDirect3DIndexBuffer9* m_IndexBuffer;

	XMMATRIX m_MatWVP;
	XMFLOAT4 m_LightVecW;
	XMFLOAT4 m_DiffuseLight;

	float m_CameraRotationY;
	float m_CameraRadius;
	float m_CameraHeight;
};

void _cdecl main()
{
	Demo_360 app;
	app.Initialize();
	if( FAILED( app.InitD3D() ) )
        return;

	app.InitScene();
	app.InitApp();
	for (;;)
	{
		app.Update();
		app.Render();
	}

}

HRESULT Demo_360::Initialize()
{
	m_CameraRadius    = 16.0f;
	m_CameraRotationY = 1.2 * D3DX_PI;
	m_CameraHeight    = 3.0f;

	return S_OK;
}

HRESULT Demo_360::InitD3D()
{
	Direct3D* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!pD3D)
		return E_FAIL;

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof( d3dpp ) );
    d3dpp.BackBufferWidth = 1280;
    d3dpp.BackBufferHeight = 720;
    d3dpp.BackBufferFormat =  ( D3DFORMAT )MAKESRGBFMT( D3DFMT_A8R8G8B8 );
    d3dpp.FrontBufferFormat = ( D3DFORMAT )MAKESRGBFMT( D3DFMT_LE_X8R8G8B8 );
    d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
    d3dpp.MultiSampleQuality = 0;
    d3dpp.BackBufferCount = 1;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	    // Create the Direct3D device.
    if( FAILED( pD3D->CreateDevice( 0, D3DDEVTYPE_HAL, NULL,
                                    D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                    &d3dpp, &g_pd3dDevice ) ) )
        return E_FAIL;

    return S_OK;
}

HRESULT Demo_360::InitScene()
{

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

	HRESULT hr;
	//
	//    // Create the box vertex shader
 //   if( hr = ATG::LoadVertexShader( "game:\\Shaders\\FloatDepthVS.xvu", &m_pBoxVS ) ) 
 //   {
 //       ATG_PrintError( "Couldn't create FloatDepthVS.xvu\n" );
 //   }

 //   // Create the box pixel shader
 //   if( hr = ATG::LoadPixelShader( "game:\\Shaders\\FloatDepthPS.xpu", &m_pBoxPS ) ) 
 //   {
 //       ATG_PrintError( "Couldn't create FloatDepthPS.xpu\n" );
 //   }


// Structure to hold vertex data

	ID3DXBuffer* pShaderCode = NULL;
    ID3DXBuffer* pErrorMsg = NULL;

	 // Compile vertex shader
   hr = D3DXCompileShader( g_strVertexShaderProgram, ( UINT )strlen( g_strVertexShaderProgram ),
                                    NULL, NULL, "main", "vs_2_0", 0,
                                    &pShaderCode, &pErrorMsg, NULL );
    if( FAILED( hr ) )
    {
        OutputDebugStringA( pErrorMsg ? ( CHAR* )pErrorMsg->GetBufferPointer() : "" );
        exit( 1 );
    }

    // Create vertex shader
  
    g_pd3dDevice->CreateVertexShader( ( DWORD* )pShaderCode->GetBufferPointer(),
                                      &m_pBoxVS );

    // Shader code is no longer required
    pShaderCode->Release();
    pShaderCode = NULL;

    // Compile pixel shader
    hr = D3DXCompileShader( g_strPixelShaderProgram, ( UINT )strlen( g_strPixelShaderProgram ),
                            NULL, NULL, "main", "ps_2_0", 0,
                            &pShaderCode, &pErrorMsg, NULL );
    if( FAILED( hr ) )
    {
        OutputDebugStringA( pErrorMsg ? ( CHAR* )pErrorMsg->GetBufferPointer() : "" );
        exit( 1 );
    }


    g_pd3dDevice->CreatePixelShader( ( DWORD* )pShaderCode->GetBufferPointer(),
                                     &m_pBoxPS );

    // Shader code no longer required
    pShaderCode->Release();
    pShaderCode = NULL;

    // Load texture

    // D3DXCreateTextureFromFile loads a texture and can optionally resize it,
    // filter it, generate mip levels, etc.  It is good for game prototyping
    // but is not suitable for final shipping code due to load time performance
    // reasons.
    hr = D3DXCreateTextureFromFileEx( g_pd3dDevice, "game:\\Media\\Textures\\crate.tga",
                                      D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
                                      0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT,
                                      D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL,
                                      &m_Texture );
    if( FAILED( hr ) )
	{
		char buffer[256];
		sprintf( buffer, "Texture load failed. HRESULT = 0x%08X\n", hr );
	    OutputDebugStringA( buffer );
		DebugBreak();

	}
    // Make texture format sRGB, since the source image is encoded in sRGB space.
    // Note: this is not the best quality way of doing this - see the ATG::ConvertTextureToGoodSRGB 
    // function in the ATG framework for the full high-quality method.
    m_Texture->Format.SignX = GPUSIGN_GAMMA;
    m_Texture->Format.SignY = GPUSIGN_GAMMA;
    m_Texture->Format.SignZ = GPUSIGN_GAMMA;

// Create and initialize vertex buffers
     g_pd3dDevice->CreateVertexBuffer( sizeof( g_BoxVertices ),
                                                  D3DUSAGE_WRITEONLY,
                                                  NULL,
                                                  D3DPOOL_DEFAULT,
                                                  &m_pInnerBoxVB,
                                                  NULL ) ;

    // Put the data for the patches into our vertex buffer.
    BOXVERTEX* pVertices;


    m_pInnerBoxVB->Lock( 0, 0, ( VOID** )&pVertices, 0 );
    for( UINT i = 0; i < 4 * 6; i++ )
    {
        // Scale position
        XMVECTOR Position = XMLoadFloat3( &g_BoxVertices[i].Position );
        Position = Position;
        XMStoreVector3( &pVertices[i].Position, Position );

        pVertices[i].Normal = g_BoxVertices[i].Normal;

		pVertices[i].UV = g_BoxVertices[i].UV;
    }

    m_pInnerBoxVB->Unlock();

    // Define the vertex elements
    static const D3DVERTEXELEMENT9 VertexElements[4] =
    {
        { 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		 { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        D3DDECL_END()
    };

    g_pd3dDevice->CreateVertexDeclaration( VertexElements, &m_VertexDecl );

	g_matWorld = XMMatrixIdentity();

	// Projection Matrix
	FLOAT fAspectRatio = ( FLOAT )m_d3dpp.BackBufferWidth / ( FLOAT )m_d3dpp.BackBufferHeight;

    XMVECTOR m_vEye = XMVectorSet( 0.0f, 3.0f, -27.0f, 0.0f );
    XMVECTOR m_vLookAt = XMVectorSet( 0.0f, 0.0f, 0.0f, 0.0f );
    XMVECTOR m_vUp = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );

    // Set the transform matrices
    g_matWorld = XMMatrixIdentity();
    g_matView = XMMatrixLookAtLH( m_vEye, m_vLookAt, m_vUp );
    g_matProj = XMMatrixPerspectiveFovLH( XM_PI / 4, fAspectRatio, 0.01f, 100.0f );

	g_matWorld2 = XMMatrixIdentity();
	XMMATRIX Translation = XMMatrixTranslation(0.0f, -7.0f, 0.0f);
	g_matWorld2 = g_matWorld2 * Translation;
	XMMATRIX Scaling = XMMatrixScaling(20.0f, 1.0f, 20.0f);
	g_matWorld2 = g_matWorld2 * Scaling;

	m_LightVecW = XMFLOAT4(-0.5f, 0.75f, -2.0f, 0.0f);
	m_DiffuseLight = XMFLOAT4(0.8f, 0.1f, 0.1f, 1.0f);

	return S_OK;
}

HRESULT Demo_360::Update()
{
	// Get the current gamepad status
	m_pGamepad = ATG::Input::GetMergedInput();

	if( m_pGamepad->wPressedButtons & XINPUT_GAMEPAD_DPAD_UP )
	{
            m_CameraHeight   += 25.0f * dt;
	}

	if( m_pGamepad->wPressedButtons & XINPUT_GAMEPAD_DPAD_DOWN )
	{
            m_CameraHeight   -= 25.0f * dt;
	}

	if( m_pGamepad->wPressedButtons & XINPUT_GAMEPAD_DPAD_LEFT )
	{
            m_CameraRotationY   += 25.0f * dt;
	}

	if( m_pGamepad->wPressedButtons & XINPUT_GAMEPAD_DPAD_RIGHT )
	{
            m_CameraRotationY   -= 25.0f * dt;
	}

		// If we rotate over 360 degrees, just roll back to 0
	if( fabsf(m_CameraRotationY) >= 2.0f * D3DX_PI ) 
		m_CameraRotationY = 0.0f;

	// Don't let radius get too small.
	if( m_CameraRadius < 5.0f )
		m_CameraRadius = 5.0f;

	BuildViewMatrix();

	return S_OK;
}

HRESULT Demo_360::Render()
{
		g_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, D3DCOLOR_XRGB(0,0,255), 1.0f, 0L);

		g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
		g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
		g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

		// Set shaders
        g_pd3dDevice->SetVertexShader( m_pBoxVS );
        g_pd3dDevice->SetPixelShader( m_pBoxPS );

		g_pd3dDevice->SetStreamSource( 0, m_pInnerBoxVB, 0, sizeof( BOXVERTEX ) );
	

		// Set the vertex declaration
		g_pd3dDevice->SetVertexDeclaration( m_VertexDecl );

		// Configure sampler 0 for trilinear sampling
		g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
		g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

		// Set texture 0
		g_pd3dDevice->SetTexture( 0, m_Texture );

		// Set the stream source
	

		// Set the index buffer


				    // Setup the vertex shader inputs.
		XMMATRIX matWVP = g_matWorld * g_matView * g_matProj;
		matWVP = XMMatrixTranspose( matWVP );
		g_InvWorld = XMMatrixInverse(&XMMatrixDeterminant(g_matWorld), g_matWorld);
		g_InvWorld = XMMatrixTranspose(g_InvWorld);
		g_InvWorld2 = XMMatrixInverse(&XMMatrixDeterminant(g_matWorld2), g_matWorld2);
		g_InvWorld2 = XMMatrixTranspose(g_InvWorld2);
        // Set shader constants
        g_pd3dDevice->SetVertexShaderConstantF( 0, ( FLOAT* )&matWVP, 4 );
		g_pd3dDevice->SetVertexShaderConstantF(4, (FLOAT*)&m_LightVecW, 1);
		g_pd3dDevice->SetVertexShaderConstantF(5, (FLOAT*)&m_DiffuseLight, 1);
		g_pd3dDevice->SetVertexShaderConstantF( 6, ( FLOAT* )&g_InvWorld, 4 );
		g_pd3dDevice->DrawPrimitive( D3DPT_QUADLIST, 0, 6 );

		// Setup the vertex shader inputs

		XMMATRIX matWVP2 = g_matWorld2 * g_matView * g_matProj;
		matWVP2 = XMMatrixTranspose( matWVP2 );
        // Set shader constants
        g_pd3dDevice->SetVertexShaderConstantF( 0, ( FLOAT* )&matWVP2, 4 );
		g_pd3dDevice->SetVertexShaderConstantF( 6, ( FLOAT* )&g_InvWorld2, 4 );
		g_pd3dDevice->DrawPrimitive( D3DPT_QUADLIST, 0, 6 );

		g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

		return S_OK;
}

void Demo_360::BuildViewMatrix()
{
	float x = m_CameraRadius * cosf(m_CameraRotationY);
	float z = m_CameraRadius * sinf(m_CameraRotationY);

	FLOAT fAspectRatio = ( FLOAT )m_d3dpp.BackBufferWidth / ( FLOAT )m_d3dpp.BackBufferHeight;

    XMVECTOR m_vEye = XMVectorSet( x, m_CameraHeight, z, 0.0f );
    XMVECTOR m_vLookAt = XMVectorSet( 0.0f, 0.0f, 0.0f, 0.0f );
    XMVECTOR m_vUp = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );

    g_matView = XMMatrixLookAtLH( m_vEye, m_vLookAt, m_vUp );

	g_matProj = XMMatrixPerspectiveFovLH( XM_PI / 4, fAspectRatio, 0.01f, 5000.0f );
}