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

#include <AtgSceneAll.h>
#include "Vertex.h"

const CHAR* g_strShadowVS =
    "float4x4 gWorldLightViewProj : register(c0); "
    "struct VS_IN { float3 Pos : POSITION0; };     "
    "struct VS_OUT                                "
    "{                                            "
    "    float4 Pos : POSITION0;                  "
    "    float Depth : TEXCOORD0;                 "
    "};                                           "
    "VS_OUT main(VS_IN In)                        "
    "{                                            "
    "    VS_OUT Out;                              "
    "    Out.Pos = mul(float4(In.Pos, 1.0f), gWorldLightViewProj); "
    "    Out.Depth = Out.Pos.z / Out.Pos.w;       "
    "    return Out;                              "
    "}                                            ";

const CHAR* g_strShadowPS =
    "struct PS_IN                                 "
    "{                                            "
    "    float4 Pos : POSITION0;                  "
    "    float Depth : TEXCOORD0;                 "
    "};                                           "
    "float4 main(PS_IN In) : COLOR                "
    "{                                            "
    "    return float4(In.Depth, In.Depth, In.Depth, 1.0f); "
    "}                                            ";
//--------------------------------------------------------------------------------------
// Vertex shader
//--------------------------------------------------------------------------------------
const CHAR* g_strVertexShaderProgram =
    "float4x4 matWVP       : register(c0);      "
    "float4x4 g_InvWorld   : register(c6);      "
    "float4x4 gWorld       : register(c14);     "
	"float4x4 gWorldLightViewProj : register(c22);"
    "struct VS_IN                              "
    "{                                         "
    "    float3 ObjPos : POSITION0;            "
    "    float3 Norm   : NORMAL0;              "
    "    float2 UV     : TEXCOORD0;            "
	"	 float3 Tangent : TANGENT0;			   "
    "};                                        "

    "struct VS_OUT                             "
    "{                                         "
    "    float4 ProjPos : POSITION0;           "
    "    float2 UV      : TEXCOORD0;           "
    "    float3 PosW    : TEXCOORD1;           "
    "    float3 NormalW : TEXCOORD2;           "
	"	 float3 TangentW :TEXCOORD3;           "
	"	 float4 ShadowPos : TEXCOORD4;"
    "};                                        "

    "VS_OUT main(VS_IN In)                     "
    "{                                         "
    "    VS_OUT Out;                           "
    "    Out.ProjPos = mul(float4(In.ObjPos, 1.0f), matWVP); "
    "    Out.PosW = mul(float4(In.ObjPos, 1.0f), gWorld).xyz; "
    "    Out.NormalW = normalize(mul(float4(In.Norm, 0.0f), g_InvWorld).xyz); "
	"    Out.TangentW = normalize(mul(float4(In.Tangent, 0.0f), gWorld).xyz);"
    "    Out.UV = In.UV;                       "
	"    Out.ShadowPos = mul(float4(In.ObjPos, 1.0f), gWorldLightViewProj);"
    "    return Out;                           "
    "}                                         ";


//--------------------------------------------------------------------------------------
// Pixel shader
//--------------------------------------------------------------------------------------
const CHAR* g_strPixelShaderProgram =
    "sampler2D ColorTexture : register(s0);     "
	"sampler2D NormalTexture : register(s1);	"
	"sampler2D ShadowMap : register(s2);		"
    "float4 gLightVecW      : register(c4);     "
    "float4 gDiffuseLight   : register(c5);     "
    "float4 gSpecularMtrl   : register(c10);    "
    "float4 gSpecularLight  : register(c11);    "
    "float4 gSpecularPower  : register(c12);    "
    "float4 gEyePosW        : register(c13);    "
    "float4 gDiffuseMtrl    : register(c18);    "
	"float4 g_RenderToggles : register(c26);"
    "struct PS_IN                              "
    "{                                         "
    "    float4 ProjPos : POSITION0;           "
    "    float2 UV      : TEXCOORD0;           "
    "    float3 PosW    : TEXCOORD1;           "
    "    float3 NormalW : TEXCOORD2;           "
	"    float3 TangentW : TEXCOORD3;"
	"float4 ShadowPos : TEXCOORD4;"
    "};                                        "

    "float4 main(PS_IN In) : COLOR             "
    "{                                         "
    "    float3 texColor = tex2D(ColorTexture, In.UV).rgb; "

    "    float3 N = normalize(In.NormalW);      "
	"    float3 T = normalize(In.TangentW);     "
	"    T = normalize(T - N * dot(T, N));      "
    "    float3 B = cross(T, N);                "

    "    float3 normalTex = tex2D(NormalTexture, In.UV).rgb; "
    "    normalTex = normalTex * 2.0f - 1.0f;   "
	"	 normalTex.xy *= 4.0f;"
	"    normalTex.y = -normalTex.y;"
	"	 normalTex = normalize(normalTex);"
    "    float3 mappedNormal = normalize(       "
    "        normalTex.x * T +                 "
    "        normalTex.y * B +                 "
    "        normalTex.z * N);                 "
	"    if (g_RenderToggles.x > 0.5f)"
	" {"
    "    N = mappedNormal;                      "
	" }"
    "    float3 L = normalize(gLightVecW.xyz);  "
    "    float3 V = normalize(gEyePosW.xyz - In.PosW); "
    "    float3 H = normalize(L + V);           "

    "    float ndotl = max(dot(N, L), 0.0f);    "

	"    float3 ambient = float3(0.13f, 0.13f, 0.23f) * texColor * gDiffuseMtrl.rgb; "
    "    float3 diffuse = ndotl * texColor * gDiffuseMtrl.rgb * gDiffuseLight.rgb; "

    "    float specAmount = pow(max(dot(N, H), 0.0f), gSpecularPower.r); "
    "    specAmount *= ndotl;                   "
    "    float3 specular = specAmount * gSpecularMtrl.rgb * gSpecularLight.rgb; "
	"float3 shadowProj = In.ShadowPos.xyz / In.ShadowPos.w; "
	"float2 shadowUV;"
	"shadowUV.x = shadowProj.x * 0.5f + 0.5f;"
	"shadowUV.y = -shadowProj.y * 0.5f + 0.5f;"
	"float currentDepth = shadowProj.z;"
	"float shadowDepth = tex2D(ShadowMap, shadowUV).r;"
	"float bias = 0.005f;"
	"float shadow = 1.0f;"
	"if (shadowUV.x >= 0.0f && shadowUV.x <= 1.0f &&"
		"shadowUV.y >= 0.0f && shadowUV.y <= 1.0f &&"
		"currentDepth >= 0.0f && currentDepth <= 1.0f)"
	"{"
	 "   float shadowDepth = tex2D(ShadowMap, shadowUV).r;"
		"float bias = 0.005f;"
		"if (currentDepth - bias > shadowDepth)"
		"{"
		 "   shadow = 0.35f;"
		"}"
	"}"
	" if (g_RenderToggles.y < 0.5f)"
	"{"
	" shadow = 1.0f;"
	"}"
	"    float3 finalCol = ambient + shadow * (diffuse + specular); "
    "    finalCol = saturate(finalCol);          "

    "    return float4(finalCol, 1.0f);          "
    "}                                         ";

//-------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------
D3DDevice*             m_pd3dDevice;    // Our rendering device


XMMATRIX g_matWorld;
XMMATRIX g_matWorld2;
XMMATRIX g_matProj;
XMMATRIX g_matView;
XMMATRIX g_InvWorld;
XMMATRIX g_InvWorld2;
XMMATRIX g_MatWVP;
XMMATRIX g_MatWVP2;
XMFLOAT4 g_RenderToggles;

const DWORD g_dwRigidVertexSize = 24;

// We will be double-buffering all resources.
const DWORD g_dwBufferCount = 2;
enum SkinningMethods
{
    SM_VertexShaderShadowedConstants = 0,
    SM_VertexShaderConstantBuffer,
    SM_VertexShaderVFetchVCache,
    SM_VertexShaderVFetchTCache,
    SM_VertexShaderTFetch,
    SM_VertexShaderMemExport,
    SM_VMX128,
    SM_SIZEOF
};
BOOL g_bWidescreen = TRUE;
const int SHADOW_SIZE = 256;

struct TimeInfo
{    
    LARGE_INTEGER qwTime;    
    LARGE_INTEGER qwAppTime;   

    float fAppTime;    
    float fElapsedTime;    

    float fSecsPerTick;    
};

TimeInfo g_Time;
float dt = 1.0f / 60.0f;

//--------------------------------------------------------------------------------------
// Name: struct ModelInfo
// Desc: Contains all relevant information about a single model.  Each ModelInfo
//       contains either a valid pSkinnedMesh or pStaticMesh pointer.  If the pStaticMesh
//       pointer is valid, much of the structure will not be filled in.
//--------------------------------------------------------------------------------------
struct ModelInfo
{
    // The model from the scene hierarchy
    ATG::Model* pModel;

    // Cached information about the model
    D3DVertexBuffer* pMeshVB;
    D3DIndexBuffer* pMeshIB;
    D3DVertexDeclaration* pMeshDecl;
    DWORD dwMeshVertexCount;
    DWORD dwMeshVertexStride;
    DWORD dwMeshSubsetCount;

    // The skinned mesh, and a binding that maps the skeleton instance to this mesh
    ATG::SkinnedMesh* pSkinnedMesh;
    DWORD dwSkeletonInstanceToSkinnedMeshBinding;

    // The static mesh, and a binding that maps a skeleton bone to this mesh
    ATG::StaticMesh* pStaticMesh;
    DWORD dwSkeletonBoneToStaticMeshBinding;

    // The following members are only used when pStaticMesh is NULL and pSkinnedMesh is
    // not NULL.

    // Rigid mesh vertex buffers for VMX128 skinning and memory export
    D3DVertexBuffer* pRigidMeshVB[ g_dwBufferCount ];
    D3DVertexDeclaration* pRigidMeshDecl;

    // Memory export vertex buffer (this will point to pRigidMeshVB[0])
    D3DVertexBuffer* pMemoryExportVB;

    // Memory export constants
    GPU_MEMEXPORT_STREAM_CONSTANT ExportConstantPosition;
    GPU_MEMEXPORT_STREAM_CONSTANT ExportConstantNormalBinormalTangent;
    GPU_MEMEXPORT_STREAM_CONSTANT ExportConstantTexCoord0;

    // Memory allocations for bone matrix constants
    VOID* pBoneMatrixBufferPhysical[ g_dwBufferCount ];
    VOID* pBoneMatrixBufferVirtual;

    // Vertex buffers and a vertex decl for bone matrix constants
    D3DVertexBuffer* pBoneMatrixVB[ g_dwBufferCount ];
    D3DVertexDeclaration* pBoneMatrixVBDecl;

    // Textures for bone matrix constants
    D3DTexture* pBoneMatrixTexture[ g_dwBufferCount ];

    // Constant buffers for bone matrix constants
    D3DConstantBuffer* pBoneMatrixConstantBuffer[ g_dwBufferCount ];
};

//--------------------------------------------------------------------------------------
// Globals variables and definitions
//--------------------------------------------------------------------------------------
// Structure to hold vertex data.
struct BOXVERTEX
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
	XMFLOAT2 UV;
	XMFLOAT3 Tangent;
};

//Unit Box
BOXVERTEX g_BoxVertices[4*6] =
{
    // Front (Z-)
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3( 0.0f, 0.0f,-1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3( 1.0f, 0.0f, 0.0f) },
    { XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT3( 0.0f, 0.0f,-1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3( 1.0f, 0.0f, 0.0f) },
    { XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT3( 0.0f, 0.0f,-1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3( 1.0f, 0.0f, 0.0f) },
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3( 0.0f, 0.0f,-1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3( 1.0f, 0.0f, 0.0f) },

    // Back (Z+)
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3( 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
    { XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT3( 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
    { XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT3( 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3( 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

    // Left (X-)
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },

    // Right (X+)
    { XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT3( 1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f,-1.0f) },
    { XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT3( 1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f,-1.0f) },
    { XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT3( 1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f,-1.0f) },
    { XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT3( 1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f,-1.0f) },

    // Bottom (Y-)
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3( 0.0f,-1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
    { XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT3( 0.0f,-1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
    { XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT3( 0.0f,-1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3( 0.0f,-1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },

    // Top (Y+)
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3( 0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
    { XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT3( 0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
    { XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT3( 0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3( 0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
};

// Main cube / hero crate
XMMATRIX g_matCrate0 =
    XMMatrixScaling(1.5f, 1.5f, 1.5f) *
    XMMatrixRotationY(0.25f) *
    XMMatrixTranslation(0.0f, -5.5f, 0.0f);

// Small crate front-left
XMMATRIX g_matCrate1 =
    XMMatrixScaling(1.0f, 1.0f, 1.0f) *
    XMMatrixRotationY(-0.6f) *
    XMMatrixTranslation(-5.0f, -6.0f, 3.0f);

// Tall crate / pillar
XMMATRIX g_matCrate2 =
    XMMatrixScaling(1.0f, 2.5f, 1.0f) *
    XMMatrixRotationY(0.1f) *
    XMMatrixTranslation(4.5f, -4.5f, 2.5f);

// Low wide block
XMMATRIX g_matCrate3 =
    XMMatrixScaling(2.5f, 0.6f, 1.2f) *
    XMMatrixRotationY(0.9f) *
    XMMatrixTranslation(-3.5f, -6.4f, -4.0f);

// Back-right stack bottom
XMMATRIX g_matCrate4 =
    XMMatrixScaling(1.2f, 1.2f, 1.2f) *
    XMMatrixRotationY(-0.2f) *
    XMMatrixTranslation(5.5f, -5.8f, -4.5f);

// Back-right stack top
XMMATRIX g_matCrate5 =
    XMMatrixScaling(0.9f, 0.9f, 0.9f) *
    XMMatrixRotationY(0.7f) *
    XMMatrixTranslation(5.5f, -3.7f, -4.5f);

// Long wall segment left
XMMATRIX g_matCrate6 =
    XMMatrixScaling(0.7f, 1.2f, 3.5f) *
    XMMatrixRotationY(0.15f) *
    XMMatrixTranslation(-8.0f, -5.8f, -1.0f);

// Long wall segment rear
XMMATRIX g_matCrate7 =
    XMMatrixScaling(4.0f, 1.0f, 0.7f) *
    XMMatrixRotationY(-0.1f) *
    XMMatrixTranslation(0.0f, -6.0f, -8.0f);

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
	virtual HRESULT InitScene();
    virtual HRESULT Update();
    virtual HRESULT Render();

	void BuildViewMatrix();
	void RenderShadowMap();

	// Vertex buffers
	LPDIRECT3DVERTEXBUFFER9 m_pInnerBoxVB;
	LPDIRECT3DVERTEXBUFFER9 m_pOuterBoxVB;

	// Shaders
	LPDIRECT3DVERTEXSHADER9 m_pBoxVS;
	LPDIRECT3DPIXELSHADER9 m_pBoxPS;
	LPDIRECT3DVERTEXSHADER9 m_pShadowVS;
	LPDIRECT3DPIXELSHADER9 m_pShadowPS;

	IDirect3DTexture9* m_Texture;
	IDirect3DTexture9* m_Texture1;
	IDirect3DTexture9* m_Texture2;
	IDirect3DTexture9* m_Texture3;
	IDirect3DVertexDeclaration9* m_VertexDecl;
	IDirect3DVertexShader9* m_VertexShader;
	IDirect3DPixelShader9* m_PixelShader;
	IDirect3DIndexBuffer9* m_IndexBuffer;

	XMMATRIX m_MatWVP;
	XMFLOAT4 m_LightVecW;
	XMFLOAT4 m_DiffuseLight;
	XMFLOAT4 m_SpecularLight;

	XMFLOAT4 m_DiffuseMtrl;
	XMFLOAT4 m_SpecularMtrl; 
	XMFLOAT4 m_SpecularPower;

	XMFLOAT4 m_DiffuseMtrl2;
	XMFLOAT4 m_SpecularMtrl2; 
	XMFLOAT4 m_SpecularPower2;

	float m_CameraRotationY;
	float m_CameraRadius;
	float m_CameraHeight;

	XMFLOAT4 m_vEye;

	LPDIRECT3DTEXTURE9        m_pShadowMap;
	LPDIRECT3DSURFACE9        m_pShadowSurface;
	LPDIRECT3DSURFACE9        m_pOldDepthSurface;
	LPDIRECT3DSURFACE9        m_pOldRenderTarget;

	XMVECTOR lightDir;
					 ;
	XMVECTOR lightPos;
	XMVECTOR target;
	XMVECTOR up;

	XMMATRIX lightView;
	XMMATRIX lightProj;

	XMMATRIX lightViewProj;

	// Model info structs
    ModelInfo* m_pModelInfos;
    DWORD m_dwModelCount;

    // Animation members
    ATG::Skeleton m_Skeleton;
    ATG::SkeletonInstance m_SkeletonInstance;
    BOOL m_bUpdateAnimation;
    DWORD m_dwForceUpdateCount;

    // Rendering members
    SkinningMethods m_SkinningMethod;
    D3DVertexShader* m_pVertexShaderSkinningConstants;
    D3DVertexShader* m_pVertexShaderSkinningVertexFetch;
    D3DVertexShader* m_pVertexShaderSkinningVertexFetchTextureCache;
    D3DVertexShader* m_pVertexShaderSkinningTextureFetch;
    D3DVertexShader* m_pVertexShaderSkinningMemExport;
    D3DVertexShader* m_pVertexShaderTransform;
    D3DPixelShader* m_pPixelShaderSolidColor;
    D3DPixelShader* m_pPixelShaderNormalMapping;
    DWORD m_dwFrameCount;
    XMFLOAT4 m_DirectionalLightDirection;
    XMFLOAT4 m_DirectionalLightColor;
    XMFLOAT4 m_AmbientColor;
    DWORD m_dwInstanceCount;
    BOOL m_bNoPixelShading;
    DWORD m_dwDefaultPredicationMask;
    BOOL m_bDrawScene;
    BOOL m_bDrawSkeleton;
    FLOAT m_fBoneRadius;
    D3DVIEWPORT9 m_NullViewport;
	ATG::Scene* m_pScene;

//	VOID    InitializeAnimation();
    VOID    InitializeMemExportConstants( ModelInfo* pModelInfo );

 //   VOID    SetupMaterial( ModelInfo* pModelInfo, DWORD dwSubsetIndex );
    VOID    RenderSkinnedModel( ModelInfo* pModelInfo );
    VOID    RenderStaticModel( ModelInfo* pModelInfo );
    VOID    RenderSkeleton();
	ID3DXMesh* mBoneMesh;
	std::vector<IDirect3DTexture9*> mTex;

	ATG::Mesh m_Mesh;
	ATG::Mesh m_Sphere;
	XMMATRIX worldATGMesh;

	void LoadXFile(
	const std::string& filename, 
	ID3DXMesh** meshOut,
	std::vector<IDirect3DTexture9*>& texs);
};

// Initialize static variables.
IDirect3DVertexDeclaration9* VertexPos::Decl = 0;
IDirect3DVertexDeclaration9* VertexCol::Decl = 0;
IDirect3DVertexDeclaration9* VertexPN::Decl  = 0;
IDirect3DVertexDeclaration9* VertexPNT::Decl = 0;

void InitAllVertexDeclarations()
{
	//===============================================================
	// VertexPos

	D3DVERTEXELEMENT9 VertexPosElements[] = 
	{
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		D3DDECL_END()
	};	
	m_pd3dDevice->CreateVertexDeclaration(VertexPosElements, &VertexPos::Decl);

	//===============================================================
	// VertexCol

	D3DVERTEXELEMENT9 VertexColElements[] = 
	{
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
		D3DDECL_END()
	};	
	m_pd3dDevice->CreateVertexDeclaration(VertexColElements, &VertexCol::Decl);

	//===============================================================
	// VertexPN

	D3DVERTEXELEMENT9 VertexPNElements[] = 
	{
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
		D3DDECL_END()
	};	
	m_pd3dDevice->CreateVertexDeclaration(VertexPNElements, &VertexPN::Decl);

	//===============================================================
	// VertexPNT

	D3DVERTEXELEMENT9 VertexPNTElements[] = 
	{
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
		{0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};	
	m_pd3dDevice->CreateVertexDeclaration(VertexPNTElements, &VertexPNT::Decl);
}


void _cdecl main()
{
	Demo_360 app;
	ATG::GetVideoSettings( &app.m_d3dpp.BackBufferWidth, &app.m_d3dpp.BackBufferHeight );

    // Make sure display is gamma correct.
    app.m_d3dpp.BackBufferFormat =  ( D3DFORMAT )MAKESRGBFMT( D3DFMT_A8R8G8B8 );
    app.m_d3dpp.FrontBufferFormat = ( D3DFORMAT )MAKESRGBFMT( D3DFMT_LE_X8R8G8B8 );

	app.Run();

}

HRESULT Demo_360::Initialize()
{
	//InitD3D();

	HRESULT hr;

    // Create the font
    if( FAILED( hr = m_Font.Create( "game:\\Media\\Fonts\\Arial_16.xpr" ) ) )
    {
        ATG_PrintError( "Couldn't create font\n" );
        return hr;
    }

	// Confine text drawing to the title safe area
    m_Font.SetWindow( ATG::GetTitleSafeArea() );
	m_bDrawSkeleton = FALSE;
    m_bDrawScene = TRUE;
    m_bUpdateAnimation = TRUE;
    m_dwForceUpdateCount = 0;
    m_SkinningMethod = SM_VertexShaderShadowedConstants;
    m_fBoneRadius = 0.01f;
	m_CameraRadius    = 26.0f;
	m_CameraRotationY = 1.2 * D3DX_PI;
	m_CameraHeight    = 3.0f;
	//InitScene();
	m_AppState = APPSTATE_CONTROLTEST;
    m_fDeadZone = 0.24f;  // Set default deadzone to 24%
    m_fLeftMotorSpeed = 0.0f;
    m_fRightMotorSpeed = 0.0f;

    // Quantized control values
    m_pQuantizedThumbStickValues = new BYTE[256];
    ZeroMemory( m_pQuantizedThumbStickValues, 256 );

    m_pQuantizedButtonValues = new BYTE[256];
    ZeroMemory( m_pQuantizedButtonValues, 256 );

	hr;
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
  
    m_pd3dDevice->CreateVertexShader( ( DWORD* )pShaderCode->GetBufferPointer(),
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


    m_pd3dDevice->CreatePixelShader( ( DWORD* )pShaderCode->GetBufferPointer(),
                                     &m_pBoxPS );

    // Shader code no longer required
    pShaderCode->Release();
    pShaderCode = NULL;

		 // Compile vertex shader
   hr = D3DXCompileShader( g_strShadowVS, ( UINT )strlen( g_strShadowVS  ),
                                    NULL, NULL, "main", "vs_2_0", 0,
                                    &pShaderCode, &pErrorMsg, NULL );
    if( FAILED( hr ) )
    {
        OutputDebugStringA( pErrorMsg ? ( CHAR* )pErrorMsg->GetBufferPointer() : "" );
        exit( 1 );
    }

    // Create vertex shader
  
    m_pd3dDevice->CreateVertexShader( ( DWORD* )pShaderCode->GetBufferPointer(),
                                      &m_pShadowVS );

    // Shader code is no longer required
    pShaderCode->Release();
    pShaderCode = NULL;

	hr = D3DXCompileShader( g_strShadowPS , ( UINT )strlen( g_strShadowPS  ),
                            NULL, NULL, "main", "ps_2_0", 0,
                            &pShaderCode, &pErrorMsg, NULL );
    if( FAILED( hr ) )
    {
        OutputDebugStringA( pErrorMsg ? ( CHAR* )pErrorMsg->GetBufferPointer() : "" );
        exit( 1 );
    }


    m_pd3dDevice->CreatePixelShader( ( DWORD* )pShaderCode->GetBufferPointer(),
                                     &m_pShadowPS );

    // Shader code no longer required
    pShaderCode->Release();
    pShaderCode = NULL;

    // Load texture

    // D3DXCreateTextureFromFile loads a texture and can optionally resize it,
    // filter it, generate mip levels, etc.  It is good for game prototyping
    // but is not suitable for final shipping code due to load time performance
    // reasons.
    hr = D3DXCreateTextureFromFileEx( m_pd3dDevice, "game:\\Media\\Textures\\cube_albedo.tga",
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

	    hr = D3DXCreateTextureFromFileEx( m_pd3dDevice, "game:\\Media\\Textures\\cube_normal.tga",
                                      D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
                                      0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT,
                                      D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL,
                                      &m_Texture1 );
    if( FAILED( hr ) )
	{
		char buffer[256];
		sprintf( buffer, "Texture load failed. HRESULT = 0x%08X\n", hr );
	    OutputDebugStringA( buffer );
		DebugBreak();

	}

	    // Load texture

    // D3DXCreateTextureFromFile loads a texture and can optionally resize it,
    // filter it, generate mip levels, etc.  It is good for game prototyping
    // but is not suitable for final shipping code due to load time performance
    // reasons.
    hr = D3DXCreateTextureFromFileEx( m_pd3dDevice, "game:\\Media\\Textures\\grass.tga",
                                      D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
                                      0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT,
                                      D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL,
                                      &m_Texture2 );
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
    m_Texture2->Format.SignX = GPUSIGN_GAMMA;
    m_Texture2->Format.SignY = GPUSIGN_GAMMA;
    m_Texture2->Format.SignZ = GPUSIGN_GAMMA;

	    hr = D3DXCreateTextureFromFileEx( m_pd3dDevice, "game:\\Media\\Textures\\ground_normal.tga",
                                      D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
                                      0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT,
                                      D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL,
                                      &m_Texture3 );
    if( FAILED( hr ) )
	{
		char buffer[256];
		sprintf( buffer, "Texture load failed. HRESULT = 0x%08X\n", hr );
	    OutputDebugStringA( buffer );
		DebugBreak();

	}

	// Create scene object.
    m_pScene = new ATG::Scene();
    ATG::ResourceDatabase* pRDB = m_pScene->GetResourceDatabase();

    // Create default textures in the resource database.
    pRDB->CreateDefaultResources();

	// Load character and animation data from a scene file.
    ATG::SceneFileParser::LoadXATGFile( "game:\\media\\scenes\\bone.x", m_pScene, NULL,
                                        ATG::XATGLOADER_DONOTINITIALIZEMATERIALS, NULL );


// Create and initialize vertex buffers
     m_pd3dDevice->CreateVertexBuffer( sizeof( g_BoxVertices ),
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
		pVertices[i].Tangent = g_BoxVertices[i].Tangent;
    }

    m_pInnerBoxVB->Unlock();

    // Define the vertex elements
    static const D3DVERTEXELEMENT9 VertexElements[5] =
    {
        { 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		 { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		 { 0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0 },
        D3DDECL_END()
    };

    m_pd3dDevice->CreateVertexDeclaration( VertexElements, &m_VertexDecl );

	g_matWorld = XMMatrixIdentity();

	// Projection Matrix
	FLOAT fAspectRatio = ( FLOAT )m_d3dpp.BackBufferWidth / ( FLOAT )m_d3dpp.BackBufferHeight;

	m_vEye = XMFLOAT4( 0.0f, 3.0f, -27.0f, 0.0f );
    XMVECTOR m_vLookAt = XMVectorSet( 0.0f, 0.0f, 0.0f, 0.0f );
    XMVECTOR m_vUp = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );

    // Set the transform matrices
    g_matWorld = XMMatrixIdentity();
    g_matView = XMMatrixLookAtLH( XMVectorSet(m_vEye.x, m_vEye.y, m_vEye.z, m_vEye.w), m_vLookAt, m_vUp );
    g_matProj = XMMatrixPerspectiveFovLH( XM_PI / 4, fAspectRatio, 0.01f, 100.0f );

	g_matWorld2 = XMMatrixIdentity();
	XMMATRIX Translation = XMMatrixTranslation(0.0f, -7.0f, 0.0f);
	g_matWorld2 = g_matWorld2 * Translation;
	XMMATRIX Scaling = XMMatrixScaling(20.0f, 1.0f, 20.0f);
	g_matWorld2 = g_matWorld2 * Scaling;

	worldATGMesh = XMMatrixIdentity();
	worldATGMesh = worldATGMesh * XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	worldATGMesh = worldATGMesh * XMMatrixScaling(5.0, 5.0, 5.0);

	m_DiffuseMtrl  = XMFLOAT4(0.75f, 0.68f, 0.58f, 1.0f);
	m_SpecularMtrl = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	m_SpecularPower = XMFLOAT4(24.0f, 24.0f, 24.0f, 24.0f);

	m_DiffuseMtrl2  = XMFLOAT4(0.35f, 0.45f, 0.35f, 1.0f);
	m_SpecularMtrl2 = XMFLOAT4(0.04f, 0.04f, 0.04f, 1.0f);
	m_SpecularPower2 = XMFLOAT4(4.0f, 4.0f, 4.0f, 4.0f);

	m_LightVecW = XMFLOAT4(-0.8f, 0.4f, -0.6f, 0.0f);
	m_DiffuseLight = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
	m_SpecularLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	lightDir = XMVector3Normalize(XMLoadFloat4(&m_LightVecW));

	lightPos = lightDir * 20.0f;
	target   = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	up       = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	lightView = XMMatrixLookAtLH(lightPos, target, up);
	lightProj = XMMatrixOrthographicLH(50.0f, 50.0f, 1.0f, 120.0f);

	lightViewProj = lightView * lightProj;

	hr = S_OK;

	hr = m_pd3dDevice->CreateTexture(
		SHADOW_SIZE,
		SHADOW_SIZE,
		1,
		0,
		D3DFMT_R32F,
		D3DPOOL_DEFAULT,
		&m_pShadowMap,
		NULL);

	if (FAILED(hr))
	{
		OutputDebugStringA("Create shadow texture failed\n");
		DebugBreak();
	}

	hr = m_pd3dDevice->CreateRenderTarget(
		SHADOW_SIZE,
		SHADOW_SIZE,
		D3DFMT_R32F,
		D3DMULTISAMPLE_NONE,
		0,
		0,
		&m_pShadowSurface,
		NULL);

	if (FAILED(hr))
	{
		OutputDebugStringA("Create shadow render target failed\n");
		DebugBreak();
	}

	g_RenderToggles = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	 // Search for 4 specific models in the scene file.
    const WCHAR* strModelNames[] = { L"body", L"Head", L"L_eyeBall", L"R_eyeBall" };
    m_dwModelCount = ARRAYSIZE( strModelNames );
    m_pModelInfos = new ModelInfo[ m_dwModelCount ];
    ZeroMemory( m_pModelInfos, m_dwModelCount * sizeof( ModelInfo ) );
	m_Mesh.Create("game:\\Media\\scenes\\bone.xbg");
	m_Sphere.Create("game:\\Media\\scenes\\sphere.xbg");
    // Initialize animation system.
  //  InitializeAnimation();

	return S_OK;
}

HRESULT Demo_360::InitScene()
{

    return S_OK;
}

HRESULT Demo_360::Update()
{
	// Get the current time
    FLOAT fTime = ( FLOAT )m_Timer.GetAppTime();

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

	if (m_pGamepad->wPressedButtons & XINPUT_GAMEPAD_X)
	{
		if (g_RenderToggles.x > 0.5f)
			g_RenderToggles.x = 0.0f;
		else
			g_RenderToggles.x = 1.0f;
	}

	if (m_pGamepad->wPressedButtons & XINPUT_GAMEPAD_A)
	{
		if (g_RenderToggles.y > 0.5f)
			g_RenderToggles.y = 0.0f;
		else
			g_RenderToggles.y = 1.0f;
	}

	// If we rotate over 360 degrees, just roll back to 0
	if( fabsf(m_CameraRotationY) >= 2.0f * D3DX_PI ) 
		m_CameraRotationY = 0.0f;

	// Don't let radius get too small.
	if( m_CameraRadius < 5.0f )
		m_CameraRadius = 5.0f;

	BuildViewMatrix();

	XMMATRIX matWVP = g_matWorld * g_matView * g_matProj;
	g_MatWVP = XMMatrixTranspose( matWVP );
	g_InvWorld = XMMatrixInverse(&XMMatrixDeterminant(g_matWorld), g_matWorld);
//	g_InvWorld = XMMatrixTranspose(g_InvWorld);
	g_InvWorld2 = XMMatrixInverse(&XMMatrixDeterminant(g_matWorld2), g_matWorld2);
//	g_InvWorld2 = XMMatrixTranspose(g_InvWorld2);
	XMMATRIX matWVP2 = g_matWorld2 * g_matView * g_matProj;
	g_MatWVP2 = XMMatrixTranspose( matWVP2 );

	D3DVERTEXELEMENT9 VertexPNTElements[] =
	{
		{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

HRESULT hr = m_pd3dDevice->CreateVertexDeclaration(
    VertexPNTElements,
    &VertexPNT::Decl);

if (FAILED(hr))
{
    OutputDebugStringA("Failed to create VertexPNT declaration\n");
}

	return S_OK;
}

void Demo_360::BuildViewMatrix()
{
	float x = m_CameraRadius * cosf(m_CameraRotationY);
	float z = m_CameraRadius * sinf(m_CameraRotationY);

	FLOAT fAspectRatio = ( FLOAT )m_d3dpp.BackBufferWidth / ( FLOAT )m_d3dpp.BackBufferHeight;

    XMVECTOR eye = XMVectorSet(x, m_CameraHeight, z, 1.0f);
	XMStoreFloat4(&m_vEye, eye);

	
    XMVECTOR m_vLookAt = XMVectorSet( 0.0f, 0.0f, 0.0f, 0.0f );
    XMVECTOR m_vUp = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );

    g_matView = XMMatrixLookAtLH(eye, m_vLookAt, m_vUp);

	g_matProj = XMMatrixPerspectiveFovLH( XM_PI / 4, fAspectRatio, 0.01f, 5000.0f );
}

HRESULT Demo_360::Render()
{
    struct SceneCube
    {
        XMMATRIX World;
        IDirect3DTexture9* Albedo;
        IDirect3DTexture9* Normal;
        XMFLOAT4 DiffuseMtrl;
        XMFLOAT4 SpecularMtrl;
        XMFLOAT4 SpecularPower;
    };

    // Ground top is roughly at y = -6.0 because:
    // ground scaleY = 1.0, translationY = -7.0, cube extends from -1 to +1.
    SceneCube scene[] =
    {
        // Main hero crate
        {
            XMMatrixScaling(1.5f, 1.5f, 1.5f) *
            XMMatrixRotationY(0.25f) *
            XMMatrixTranslation(0.0f, -4.5f, 0.0f),
            m_Texture, m_Texture1,
            m_DiffuseMtrl, m_SpecularMtrl, m_SpecularPower
        },

        // Small front-left crate
        {
            XMMatrixScaling(1.0f, 1.0f, 1.0f) *
            XMMatrixRotationY(-0.6f) *
            XMMatrixTranslation(-5.0f, -5.0f, 3.0f),
            m_Texture, m_Texture1,
            m_DiffuseMtrl, m_SpecularMtrl, m_SpecularPower
        },

        // Tall pillar
        {
            XMMatrixScaling(1.0f, 2.5f, 1.0f) *
            XMMatrixRotationY(0.1f) *
            XMMatrixTranslation(4.5f, -3.5f, 2.5f),
            m_Texture, m_Texture1,
            m_DiffuseMtrl, m_SpecularMtrl, m_SpecularPower
        },

        // Low wide block
        {
            XMMatrixScaling(2.5f, 0.6f, 1.2f) *
            XMMatrixRotationY(0.9f) *
            XMMatrixTranslation(-3.5f, -5.4f, -4.0f),
            m_Texture, m_Texture1,
            m_DiffuseMtrl, m_SpecularMtrl, m_SpecularPower
        },

        // Stack bottom
        {
            XMMatrixScaling(1.2f, 1.2f, 1.2f) *
            XMMatrixRotationY(-0.2f) *
            XMMatrixTranslation(5.5f, -4.8f, -4.5f),
            m_Texture, m_Texture1,
            m_DiffuseMtrl, m_SpecularMtrl, m_SpecularPower
        },

        // Stack top
        {
            XMMatrixScaling(0.9f, 0.9f, 0.9f) *
            XMMatrixRotationY(0.7f) *
            XMMatrixTranslation(5.5f, -2.7f, -4.5f),
            m_Texture, m_Texture1,
            m_DiffuseMtrl, m_SpecularMtrl, m_SpecularPower
        },

        // Left wall segment
        {
            XMMatrixScaling(0.7f, 1.2f, 3.5f) *
            XMMatrixRotationY(0.15f) *
            XMMatrixTranslation(-8.0f, -4.8f, -1.0f),
            m_Texture, m_Texture1,
            m_DiffuseMtrl, m_SpecularMtrl, m_SpecularPower
        },

        // Rear wall segment
        {
            XMMatrixScaling(4.0f, 1.0f, 0.7f) *
            XMMatrixRotationY(-0.1f) *
            XMMatrixTranslation(0.0f, -5.0f, -8.0f),
            m_Texture, m_Texture1,
            m_DiffuseMtrl, m_SpecularMtrl, m_SpecularPower
        },

        // Ground
        {
            g_matWorld2,
            m_Texture2, m_Texture3,
            m_DiffuseMtrl2, m_SpecularMtrl2, m_SpecularPower2
        }
    };

    const int sceneCount = sizeof(scene) / sizeof(scene[0]);

    // ------------------------------------------------------------
    // PASS 1: Shadow map
    // ------------------------------------------------------------
    D3DVIEWPORT9 oldViewport;
    m_pd3dDevice->GetViewport(&oldViewport);

    LPDIRECT3DSURFACE9 oldRenderTarget = NULL;
    m_pd3dDevice->GetRenderTarget(0, &oldRenderTarget);

    D3DVIEWPORT9 shadowViewport;
    shadowViewport.X = 0;
    shadowViewport.Y = 0;
    shadowViewport.Width = SHADOW_SIZE;
    shadowViewport.Height = SHADOW_SIZE;
    shadowViewport.MinZ = 0.0f;
    shadowViewport.MaxZ = 1.0f;

    m_pd3dDevice->SetRenderTarget(0, m_pShadowSurface);
    m_pd3dDevice->SetViewport(&shadowViewport);

    m_pd3dDevice->Clear(
        0,
        NULL,
        D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        0xffffffff,
        1.0f,
        0);

    m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
    m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    m_pd3dDevice->SetVertexShader(m_pShadowVS);
    m_pd3dDevice->SetPixelShader(m_pShadowPS);
    m_pd3dDevice->SetStreamSource(0, m_pInnerBoxVB, 0, sizeof(BOXVERTEX));
    m_pd3dDevice->SetVertexDeclaration(m_VertexDecl);

    m_pd3dDevice->SetTexture(0, NULL);
    m_pd3dDevice->SetTexture(1, NULL);
    m_pd3dDevice->SetTexture(2, NULL);

    for (int i = 0; i < sceneCount; ++i)
    {
        XMMATRIX shadowMtx = scene[i].World * lightViewProj;
        shadowMtx = XMMatrixTranspose(shadowMtx);

        // Shadow VS expects c0
        m_pd3dDevice->SetVertexShaderConstantF(0, (FLOAT*)&shadowMtx, 4);
        m_pd3dDevice->DrawPrimitive(D3DPT_QUADLIST, 0, 6);
    }


    XMMATRIX shadowMtx = worldATGMesh * lightViewProj;
    shadowMtx = XMMatrixTranspose(shadowMtx);


    m_pd3dDevice->SetVertexShaderConstantF(0, (FLOAT*)&shadowMtx, 4);



	m_Mesh.Render();


	m_pd3dDevice->SetPixelShaderConstantF(10, (FLOAT*)&XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f), 1);
	m_pd3dDevice->SetPixelShaderConstantF(12, (FLOAT*)&XMFLOAT4(8.0f, 8.0f, 8.0f, 8.0f), 1);
    m_pd3dDevice->SetPixelShaderConstantF(18, (FLOAT*)&XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f), 1);

	m_Sphere.Render();

    m_pd3dDevice->Resolve(
        0,
        NULL,
        m_pShadowMap,
        NULL,
        0,
        0,
        NULL,
        0.0f,
        0,
        NULL);

    m_pd3dDevice->SetRenderTarget(0, oldRenderTarget);
    m_pd3dDevice->SetViewport(&oldViewport);

    if (oldRenderTarget)
        oldRenderTarget->Release();

    // ------------------------------------------------------------
    // PASS 2: Main render
    // ------------------------------------------------------------
    m_pd3dDevice->Clear(
        0L,
        NULL,
        D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
        D3DCOLOR_XRGB(0, 0, 255),
        1.0f,
        0L);

    m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
    m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    m_pd3dDevice->SetVertexShader(m_pBoxVS);
    m_pd3dDevice->SetPixelShader(m_pBoxPS);

    m_pd3dDevice->SetStreamSource(0, m_pInnerBoxVB, 0, sizeof(BOXVERTEX));
    m_pd3dDevice->SetVertexDeclaration(m_VertexDecl);

    m_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    m_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    m_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    m_pd3dDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    m_pd3dDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    m_pd3dDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    m_pd3dDevice->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    m_pd3dDevice->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    m_pd3dDevice->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    m_pd3dDevice->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

    // Shared pixel constants
    m_pd3dDevice->SetPixelShaderConstantF(4,  (FLOAT*)&m_LightVecW, 1);
    m_pd3dDevice->SetPixelShaderConstantF(5,  (FLOAT*)&m_DiffuseLight, 1);
    m_pd3dDevice->SetPixelShaderConstantF(11, (FLOAT*)&m_SpecularLight, 1);
    m_pd3dDevice->SetPixelShaderConstantF(13, (FLOAT*)&m_vEye, 1);
    m_pd3dDevice->SetPixelShaderConstantF(26, (FLOAT*)&g_RenderToggles, 1);

    for (int i = 0; i < sceneCount; ++i)
    {
        XMMATRIX wvp = scene[i].World * g_matView * g_matProj;
        wvp = XMMatrixTranspose(wvp);

        XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(scene[i].World), scene[i].World);

        XMMATRIX shadowMtx = scene[i].World * lightViewProj;
        shadowMtx = XMMatrixTranspose(shadowMtx);

        m_pd3dDevice->SetTexture(0, scene[i].Albedo);
        m_pd3dDevice->SetTexture(1, scene[i].Normal);
        m_pd3dDevice->SetTexture(2, m_pShadowMap);

        m_pd3dDevice->SetVertexShaderConstantF(0,  (FLOAT*)&wvp, 4);
        m_pd3dDevice->SetVertexShaderConstantF(6,  (FLOAT*)&invWorld, 4);
        m_pd3dDevice->SetVertexShaderConstantF(14, (FLOAT*)&scene[i].World, 4);
        m_pd3dDevice->SetVertexShaderConstantF(22, (FLOAT*)&shadowMtx, 4);

        m_pd3dDevice->SetPixelShaderConstantF(10, (FLOAT*)&scene[i].SpecularMtrl, 1);
        m_pd3dDevice->SetPixelShaderConstantF(12, (FLOAT*)&scene[i].SpecularPower, 1);
        m_pd3dDevice->SetPixelShaderConstantF(18, (FLOAT*)&scene[i].DiffuseMtrl, 1);

        m_pd3dDevice->DrawPrimitive(D3DPT_QUADLIST, 0, 6);

    }

	
	XMMATRIX wvp = worldATGMesh * g_matView * g_matProj;
    wvp = XMMatrixTranspose(wvp);

    XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(worldATGMesh), worldATGMesh);

    shadowMtx = worldATGMesh * lightViewProj;
    shadowMtx = XMMatrixTranspose(shadowMtx);

	m_pd3dDevice->SetTexture(0, NULL);
    m_pd3dDevice->SetTexture(1, NULL);

	m_pd3dDevice->SetVertexShaderConstantF(0,  (FLOAT*)&wvp, 4);
    m_pd3dDevice->SetVertexShaderConstantF(6,  (FLOAT*)&invWorld, 4);
    m_pd3dDevice->SetVertexShaderConstantF(14, (FLOAT*)&worldATGMesh, 4);
    m_pd3dDevice->SetVertexShaderConstantF(22, (FLOAT*)&shadowMtx, 4);



	m_Mesh.Render();


	m_pd3dDevice->SetPixelShaderConstantF(10, (FLOAT*)&XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f), 1);
	m_pd3dDevice->SetPixelShaderConstantF(12, (FLOAT*)&XMFLOAT4(8.0f, 8.0f, 8.0f, 8.0f), 1);
    m_pd3dDevice->SetPixelShaderConstantF(18, (FLOAT*)&XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f), 1);

	m_Sphere.Render();
    // ------------------------------------------------------------
    // UI
    // ------------------------------------------------------------
    m_Timer.MarkFrame();

    m_Font.Begin();
    m_Font.SetScaleFactors(1.2f, 1.2f);
    m_Font.DrawText(0, 0, 0xffffffff, L"XBOX360 Demo");
    m_Font.SetScaleFactors(1.0f, 1.0f);

    m_Font.DrawText(0, 0, 0xffffff00, m_Timer.GetFrameRate(), ATGFONT_RIGHT);

    float fps = (float)_wtof(m_Timer.GetFrameRate());
    if (fps < 0.001f)
        fps = 0.001f;

    float frameTimeMs = 1000.0f / fps;

    WCHAR buffer[64];
    swprintf_s(buffer, L"Frame Time: %.2f ms", frameTimeMs);
    m_Font.DrawText(0, 20, 0xffffff00, buffer, ATGFONT_RIGHT);

    m_Font.End();

    m_pd3dDevice->Present(NULL, NULL, NULL, NULL);

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Name: RenderStaticModel()
// Desc: Renders a static (non-skinned) model.
//--------------------------------------------------------------------------------------
VOID Demo_360::RenderStaticModel( ModelInfo* pModelInfo )
{
    assert( pModelInfo->pStaticMesh != NULL );

    // Get the world transform matrix that this model is attached to.
    // The animated world transform matrix is held by the skeleton instance.
    XMMATRIX matWorld;
	matWorld = XMMatrixIdentity();


    // Set the transform only vertex shader.
    m_pd3dDevice->SetVertexShader( m_pBoxVS );

    // Compute a world * view * projection matrix for this model.
    XMMATRIX matWVP = matWorld * g_MatWVP;
    XMMATRIX matWVP_T = XMMatrixTranspose( matWVP );
    m_pd3dDevice->SetVertexShaderConstantF( 0, ( FLOAT* )&matWVP_T, 4 );

    // Render the mesh subsets.
    for( DWORD i = 0; i < pModelInfo->dwMeshSubsetCount; ++i )
    {
     //   SetupMaterial( pModelInfo, i );
        pModelInfo->pStaticMesh->RenderSubset( i, m_pd3dDevice );
    }
}

