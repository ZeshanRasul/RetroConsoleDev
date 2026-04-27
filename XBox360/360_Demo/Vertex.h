//=============================================================================
// Vertex.h by Frank Luna (C) 2005 All Rights Reserved.
//
// This file contains all of our vertex structures.  We will add to it
// as needed throughout the book.
//=============================================================================

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

#ifndef VERTEX_H
#define VERTEX_H

// Call in constructor and destructor, respectively, of derived application class.
void InitAllVertexDeclarations();
void DestroyAllVertexDeclarations();


//===============================================================
struct VertexPos
{
	VertexPos():pos(0.0f, 0.0f, 0.0f){}
	VertexPos(float x, float y, float z):pos(x,y,z){}
	VertexPos(const XMFLOAT3& v):pos(v){}

	XMFLOAT3 pos;
	static IDirect3DVertexDeclaration9* Decl;
};

//===============================================================
struct VertexCol
{
	VertexCol():pos(0.0f, 0.0f, 0.0f),col(0x00000000){}
	VertexCol(float x, float y, float z, XMFLOAT4 c):pos(x,y,z), col(c){}
	VertexCol(const XMFLOAT3& v, XMFLOAT4 c):pos(v),col(c){}

	XMFLOAT3 pos;
	XMFLOAT4    col;
	static IDirect3DVertexDeclaration9* Decl;
};

//===============================================================
struct VertexPN
{
	VertexPN()
		:pos(0.0f, 0.0f, 0.0f),
		normal(0.0f, 0.0f, 0.0f){}
	VertexPN(float x, float y, float z, 
		float nx, float ny, float nz):pos(x,y,z), normal(nx,ny,nz){}
	VertexPN(const D3DXVECTOR3& v, const D3DXVECTOR3& n)
		:pos(v),normal(n){}

	XMFLOAT3 pos;
	XMFLOAT3 normal;
	static IDirect3DVertexDeclaration9* Decl;
};

//===============================================================
struct VertexPNT
{
	VertexPNT()
		:pos(0.0f, 0.0f, 0.0f),
		normal(0.0f, 0.0f, 0.0f),
		tex0(0.0f, 0.0f){}
	VertexPNT(float x, float y, float z, 
		float nx, float ny, float nz,
		float u, float v):pos(x,y,z), normal(nx,ny,nz), tex0(u,v){}
	VertexPNT(const D3DXVECTOR3& v, const D3DXVECTOR3& n, const D3DXVECTOR2& uv)
		:pos(v),normal(n), tex0(uv){}

	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT2 tex0;

	static IDirect3DVertexDeclaration9* Decl;
};

#endif // VERTEX_H