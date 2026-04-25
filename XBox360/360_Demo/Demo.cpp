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

//--------------------------------------------------------------------------------------
// Vertex shader
// We use the register semantic here to directly define the input register
// matWVP.  Conversely, we could let the HLSL compiler decide and check the
// constant table.
//--------------------------------------------------------------------------------------
const CHAR*         g_strVertexShaderProgram =
    "                                              "
    "                                              "
    " float4x4 matWVP : register(c0);              "
    "                                              "
    " struct VS_IN                                 "
    "                                              "
    " {                                            "
    "     float4 ObjPos : POSITION;                "  // Object space position 
    "     float2 UV     : TEXCOORD;                "  // Texture coordinate
    " };                                           "
    "                                              "
    " struct VS_OUT                                "
    " {                                            "
    "     float4 ProjPos  : POSITION;              "  // Projected space position 
    "     float2 UV       : TEXCOORD0;             "
    " };                                           "
    "                                              "
    " VS_OUT main( VS_IN In )                      "
    " {                                            "
    "     VS_OUT Out;                              "
    "     Out.ProjPos = mul( matWVP, In.ObjPos );  "  // Transform vertex into 
    "     Out.UV = In.UV;                          "  // Projected space and
    "     return Out;                              "  // Transfer UVs
    " }                                            ";


//-------------------------------------------------------------------------------------
// Pixel shader
// We use the register semantic here to directly define the input sampler
// ColorTexture.  Conversely, we could let the HLSL compiler decide and check
// the constant table.
//-------------------------------------------------------------------------------------
const CHAR*         g_strPixelShaderProgram =
    " sampler2D ColorTexture : register(s0);       "
    "                                              "
    " struct PS_IN                                 "
    " {                                            "
    "     float2 UV : TEXCOORD0;                   "  // Interpolated UV from
    " };                                           "  // the vertex shader 
    "                                              "
    " float4 main( PS_IN In ) : COLOR              "
    " {                                            "
    "     return tex2D( ColorTexture, In.UV );     "  // Sample texture and output
    " }                                            ";

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
	


	IDirect3DTexture9* pTexture;
	IDirect3DVertexDeclaration9* pVertexDecl;
	IDirect3DVertexShader9* pVertexShader;
	IDirect3DPixelShader9* pPixelShader;
	// Structure to hold vertex data

	ID3DXBuffer* pShaderCode = NULL;
    ID3DXBuffer* pErrorMsg = NULL;

	 // Compile vertex shader
    HRESULT hr = D3DXCompileShader( g_strVertexShaderProgram, ( UINT )strlen( g_strVertexShaderProgram ),
                                    NULL, NULL, "main", "vs_2_0", 0,
                                    &pShaderCode, &pErrorMsg, NULL );
    if( FAILED( hr ) )
    {
        OutputDebugStringA( pErrorMsg ? ( CHAR* )pErrorMsg->GetBufferPointer() : "" );
        exit( 1 );
    }

    // Create vertex shader
  
    g_pd3dDevice->CreateVertexShader( ( DWORD* )pShaderCode->GetBufferPointer(),
                                      &pVertexShader );

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
                                     &pPixelShader );

    // Shader code no longer required
    pShaderCode->Release();
    pShaderCode = NULL;

    // Load texture

    // D3DXCreateTextureFromFile loads a texture and can optionally resize it,
    // filter it, generate mip levels, etc.  It is good for game prototyping
    // but is not suitable for final shipping code due to load time performance
    // reasons.
    hr = D3DXCreateTextureFromFileEx( g_pd3dDevice, "game:\\Media\\Textures\\Rocks.tga",
                                      D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
                                      0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT,
                                      D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL,
                                      &pTexture );
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
    pTexture->Format.SignX = GPUSIGN_GAMMA;
    pTexture->Format.SignY = GPUSIGN_GAMMA;
    pTexture->Format.SignZ = GPUSIGN_GAMMA;

    // Define some vertices to draw
	struct COLORVERTEX
    {
        FLOAT   Position[3];
        FLOAT   UV[2];
    };

    // Triangle array width and height
    const UINT Width = 35;
    const UINT Height = 35;

    // Number of primitives in stripped mesh
    const UINT NumPrimitives = Width * Height * 2 + ( Height - 1 ) * 4;

    // Number of vertices needed in the vertex buffer
    UINT NumVertices = ( Width + 1 ) * ( Height + 1 );

   	IDirect3DVertexBuffer9* pVertexBuffer;
    g_pd3dDevice->CreateVertexBuffer( NumVertices * sizeof( COLORVERTEX ),
                                      D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT,
                                      &pVertexBuffer, NULL );

    // Fill the vertex buffer
    COLORVERTEX* pVertices;
    pVertexBuffer->Lock( 0, 0, ( VOID** )&pVertices, 0 );
    for( UINT i = 0; i < Height + 1; i++ )
    {
        for( UINT j = 0; j < Width + 1; j++ )
        {
            XMVECTOR pos;
            pos.x = ( ( FLOAT )j - ( FLOAT )( Width ) / 2.0f ) * 4.0f / Width;
            pos.y = ( ( FLOAT )i - ( FLOAT )( Height ) / 2.0f ) * 4.0f / Height;
            pos.z = 0.2f * cosf( XMVector2Length( pos ).x * 5.0f );
            pVertices->Position[0] = pos.x;
            pVertices->Position[1] = pos.y;
            pVertices->Position[2] = pos.z;

            FLOAT u = ( FLOAT )( j ) / Width;
            FLOAT v = ( FLOAT )( i ) / Height;
            pVertices->UV[0] = u;
            pVertices->UV[1] = v;

            pVertices++;
        }
    }
    pVertexBuffer->Unlock();

    // Number of indices needed in the index buffer
    UINT NumIndices = ( Width + 1 ) * 2 * Height + 2 * ( Height - 1 );

    // Create an index buffer
    IDirect3DIndexBuffer9* pIndexBuffer;
    g_pd3dDevice->CreateIndexBuffer( NumIndices * sizeof( WORD ),
                                     D3DUSAGE_WRITEONLY, D3DFMT_INDEX16,
                                     D3DPOOL_DEFAULT, &pIndexBuffer, NULL );
    // Fill the index buffer
    WORD* pIndices;
    pIndexBuffer->Lock( 0, 0, ( VOID** )&pIndices, 0 );
    UINT Index = 0;
    for( INT i = 0; i < Height; i++ )
    {
        // Tri-stripped vertices
        for( INT j = 0; j < Width + 1; j++ )
        {
            pIndices[Index++] = ( WORD )( i * ( Width + 1 ) + j );
            pIndices[Index++] = ( WORD )( ( i + 1 ) * ( Width + 1 ) + j );
        }

        // Degenerate strip connectors
        if( i != Height - 1 )
        {
            pIndices[Index++] = ( WORD )( ( i + 1 ) * ( Width + 1 ) + Width );
            pIndices[Index++] = ( WORD )( ( i + 1 ) * ( Width + 1 ) );
        }
    }
    pIndexBuffer->Unlock();

    // Define the vertex elements
    static const D3DVERTEXELEMENT9 VertexElements[3] =
    {
        { 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        D3DDECL_END()
    };

    g_pd3dDevice->CreateVertexDeclaration( VertexElements, &pVertexDecl );

	g_matWorld = XMMatrixIdentity();

	// Projection Matrix
	FLOAT fAspect = ( g_bWidescreen ) ? (16.0f / 9.0f) : (4.0f / 3.0f);
	g_matProj = XMMatrixPerspectiveFovLH(XM_PIDIV4, fAspect, 1.0f, 200.0f);

	// Initialize the view matrix
    XMVECTOR vEyePt    = { 0.0f, 0.0f,-7.0f, 0.0f };
    XMVECTOR vLookatPt = { 0.0f, 0.0f, 0.0f, 0.0f };
    XMVECTOR vUp       = { 0.0f, 1.0f, 0.0f, 0.0f };
    g_matView = XMMatrixLookAtLH( vEyePt, vLookatPt, vUp );


	for (;;)
	{
		app.Update();
		g_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, D3DCOLOR_XRGB(0,0,255), 1.0f, 0L);

		// Set shaders
		g_pd3dDevice->SetVertexShader( pVertexShader );
		g_pd3dDevice->SetPixelShader( pPixelShader );

		// Set the vertex declaration
		g_pd3dDevice->SetVertexDeclaration( pVertexDecl );

		// Configure sampler 0 for trilinear sampling
		g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
		g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

		// Set texture 0
		g_pd3dDevice->SetTexture( 0, pTexture );

		// Set the stream source
		g_pd3dDevice->SetStreamSource( 0, pVertexBuffer, 0, sizeof( COLORVERTEX ) );

		// Set the index buffer
		g_pd3dDevice->SetIndices( pIndexBuffer );
    
		XMMATRIX matWVP = g_matWorld * g_matView * g_matProj;
		g_pd3dDevice->SetVertexShaderConstantF(0, (FLOAT*)&matWVP, 4);

		g_pd3dDevice->DrawIndexedPrimitive(
    D3DPT_TRIANGLESTRIP,
    0,
    0,
    NumVertices,
    0,
    NumPrimitives
);
		g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
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


	return S_OK;
}

