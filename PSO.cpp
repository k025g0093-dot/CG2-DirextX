#include "PSO.h"

// ルートシグネチャの生成
// シェーダーにどんなリソースを渡すかを定義する
ComPtr<ID3D12RootSignature> CreateRootSignature(
	ID3D12Device* device,
	HRESULT& hr) {

	// ルートシグネチャの設定
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[2] = {};
	descriptorRange[0].BaseShaderRegister = 0;//0から始まる
	descriptorRange[0].NumDescriptors = 1;//数は一つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//自動で決める].OffsetInDescriptorsFromTableStart

	descriptorRange[1].BaseShaderRegister = 1;
	descriptorRange[1].NumDescriptors = 1;//数は一つ
	descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
	descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//自動で決める].OffsetInDescriptorsFromTableStart


	// ルートパラメータの設定（シェーダーに渡すリソースの種類を定義）
	D3D12_ROOT_PARAMETER rootParameter[5] = {};
	rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // 定数バッファビューを使用
	rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // 全シェーダーから参照可能
	rootParameter[0].Descriptor.ShaderRegister = 0; // register(b0)に対応

	rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // シェーダーリソースビューを使用
	rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // 全シェーダーから参照可能
	rootParameter[1].Descriptor.ShaderRegister = 1; // register(b0)に対応

	rootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // 定数バッファビューを使用
	rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[2].DescriptorTable.pDescriptorRanges = &descriptorRange[0];
	rootParameter[2].DescriptorTable.NumDescriptorRanges = 1;

	rootParameter[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[3].Descriptor.ShaderRegister = 1;

	rootParameter[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // 定数バッファビューを使用
	rootParameter[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[4].DescriptorTable.pDescriptorRanges = &descriptorRange[1];
	rootParameter[4].DescriptorTable.NumDescriptorRanges = 1;


	descriptionRootSignature.pParameters = rootParameter;
	descriptionRootSignature.NumParameters = _countof(rootParameter);


	//Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0~1の範囲を繰り返す
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//最大LOD
	staticSamplers[0].ShaderRegister = 0;//register(s0)に対応
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダーで使用

	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);



	// ルートシグネチャをバイナリに変換
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	// L"Object3d.VS.hlsl" を L"VertexShader.hlsl" に変更
	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

	if (FAILED(hr)) {
		Log(logStream, reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	// ルートシグネチャの生成
	ComPtr<ID3D12RootSignature> rootSignature;
	hr = device->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(rootSignature.GetAddressOf()));
	assert(SUCCEEDED(hr));

	return rootSignature;
}

// インプットレイアウトの生成
// 頂点データの形式をGPUに教える
D3D12_INPUT_LAYOUT_DESC CreateLayout() {

	// 頂点データの要素定義（今回はPOSITIONのみ）
	static D3D12_INPUT_ELEMENT_DESC inputElementDescs[4] = {};
	inputElementDescs[0].SemanticName = "POSITION"; // シェーダー側のセマンティクス名
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // float4形式
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD"; // シェーダー側のセマンティクス名
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT; // float2形式
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset =
		D3D12_APPEND_ALIGNED_ELEMENT;


	inputElementDescs[3].SemanticName = "TANGENT";
	inputElementDescs[3].SemanticIndex = 0;
	inputElementDescs[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);
	return inputLayoutDesc;
}

// ブレンドステートの生成
// 色の合成方法を定義する（今回は透明度なしのシンプルな設定）
D3D12_BLEND_DESC CreateBlendState() {
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL; // 全チャンネルへの書き込みを有効化
	return blendDesc;
}

// ラスタライザステートの生成
// ポリゴンの描画方法を定義する
D3D12_RASTERIZER_DESC CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;  // 裏面を描画しない
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID; // ポリゴンを塗りつぶして描画
	return rasterizerDesc;
}

// パイプラインステートオブジェクト（PSO）の生成
// 描画に必要な全設定をまとめたオブジェクトを作る
ComPtr<ID3D12PipelineState> CreatePipelineStateDesc(
	ID3D12Device* device,
	ComPtr<ID3D12RootSignature>& rootSignature,
	HRESULT& hr)
{
	// DXCコンパイラの初期化
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	IDxcIncludeHandler* includeHandler = nullptr;
	DxcCompilerInclude(hr, dxcUtils, dxcCompiler, includeHandler);

	// シェーダーのコンパイル
	IDxcBlob* vertexShaderBlob = CompileShader(
		L"Object3d.VS.hlsl", L"vs_6_0",
		dxcUtils, dxcCompiler, includeHandler);

	IDxcBlob* pixelShaderBlob = CompileShader(
		L"Object3d.PS.hlsl", L"ps_6_0",
		dxcUtils, dxcCompiler, includeHandler);


	// 各設定の生成
	D3D12_INPUT_LAYOUT_DESC inputLayout = CreateLayout();
	D3D12_BLEND_DESC blendDesc = CreateBlendState();
	D3D12_RASTERIZER_DESC rasterizerDesc = CreateRasterizerState();

	// PSOの設定をまとめる
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};

	// ルートシグネチャの設定
	rootSignature = CreateRootSignature(device, hr);
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();

	// インプットレイアウトの設定
	graphicsPipelineStateDesc.InputLayout = inputLayout;


	// シェーダーの設定
	graphicsPipelineStateDesc.VS = {
		vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize()
	};
	graphicsPipelineStateDesc.PS = {
		pixelShaderBlob->GetBufferPointer(),
		pixelShaderBlob->GetBufferSize()
	};

	// ブレンド・ラスタライザの設定
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;

	//ステートの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//depth機能を有効にする
	depthStencilDesc.DepthEnable = true;
	//書き込みをするところ
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はlessEqual。つまり近いと描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	//DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// レンダーターゲットの設定
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// プリミティブトポロジーの設定（三角形として描画）
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// サンプリングの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// PSOの生成
	ComPtr<ID3D12PipelineState> graphicsPipelineState;
	hr = device->CreateGraphicsPipelineState(
		&graphicsPipelineStateDesc,
		IID_PPV_ARGS(graphicsPipelineState.GetAddressOf()));
	assert(SUCCEEDED(hr));

	return graphicsPipelineState;
}


//GPU用のルートシグネチャ作成とパイプラインステート

ComPtr<ID3D12RootSignature> CreateGpuDrivenRootSignature(
	ID3D12Device* device,
	HRESULT& hr
) {

	// ルートシグネチャの設定
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// t0: texture, t1: normal texture の2つだけDescriptorTable用に定義
	// t2: InstanceData は RootSRV で直接アドレスを渡すので不要
	D3D12_DESCRIPTOR_RANGE descriptorRange[2] = {};
	descriptorRange[0].BaseShaderRegister = 0; // t0 texture
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descriptorRange[1].BaseShaderRegister = 1; // t1 normal texture
	descriptorRange[1].NumDescriptors = 1;
	descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	D3D12_ROOT_PARAMETER rootParameter[6] = {};
	rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[0].Descriptor.ShaderRegister = 0; // b0 material

	// 🌟 DescriptorTableからRootSRVに変更
	// SetGraphicsRootShaderResourceView(1, m_instanceBuffer->GetGPUVirtualAddress()) で渡す
	rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameter[1].Descriptor.ShaderRegister = 2; // register(t2)
	rootParameter[1].Descriptor.RegisterSpace = 0;

	rootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[2].DescriptorTable.pDescriptorRanges = &descriptorRange[0]; // t0 texture
	rootParameter[2].DescriptorTable.NumDescriptorRanges = 1;

	rootParameter[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[3].Descriptor.ShaderRegister = 1; // b1 light

	rootParameter[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[4].DescriptorTable.pDescriptorRanges = &descriptorRange[1]; // t1 normal texture
	rootParameter[4].DescriptorTable.NumDescriptorRanges = 1;


	rootParameter[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameter[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameter[5].Constants.ShaderRegister = 2; // b2
	rootParameter[5].Constants.RegisterSpace = 0;
	rootParameter[5].Constants.Num32BitValues = 1;



	descriptionRootSignature.pParameters = rootParameter;
	descriptionRootSignature.NumParameters = _countof(rootParameter);


	//Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0~1の範囲を繰り返す
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//最大LOD
	staticSamplers[0].ShaderRegister = 0;//register(s0)に対応
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダーで使用

	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);



	// ルートシグネチャをバイナリに変換
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	// L"Object3d.VS.hlsl" を L"VertexShader.hlsl" に変更
	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

	if (FAILED(hr)) {
		Log(logStream, reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	// ルートシグネチャの生成
	ComPtr<ID3D12RootSignature> rootSignature;
	hr = device->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(rootSignature.GetAddressOf()));
	assert(SUCCEEDED(hr));

	return rootSignature;

}


ComPtr<ID3D12PipelineState> CreateGpuDrivenPipelineStateDesc(
	ID3D12Device* device,
	ComPtr<ID3D12RootSignature>& rootSignature,
	HRESULT& hr) {

	// DXCコンパイラの初期化
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	IDxcIncludeHandler* includeHandler = nullptr;
	DxcCompilerInclude(hr, dxcUtils, dxcCompiler, includeHandler);

	// シェーダーのコンパイル
	IDxcBlob* vertexShaderBlob = CompileShader(
		L"GpuDrivenObject.VS.hlsl", L"vs_6_0",
		dxcUtils, dxcCompiler, includeHandler);

	IDxcBlob* pixelShaderBlob = CompileShader(
		L"GpuDrivenObject.PS.hlsl", L"ps_6_0",
		dxcUtils, dxcCompiler, includeHandler);



	// 各設定の生成
	D3D12_INPUT_LAYOUT_DESC inputLayout = CreateLayout();
	D3D12_BLEND_DESC blendDesc = CreateBlendState();
	D3D12_RASTERIZER_DESC rasterizerDesc = CreateRasterizerState();

	// PSOの設定をまとめる
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};

	// ルートシグネチャの設定
	rootSignature = CreateGpuDrivenRootSignature(device, hr);
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();

	// インプットレイアウトの設定
	graphicsPipelineStateDesc.InputLayout = inputLayout;


	// シェーダーの設定
	graphicsPipelineStateDesc.VS = {
		vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize()
	};
	graphicsPipelineStateDesc.PS = {
		pixelShaderBlob->GetBufferPointer(),
		pixelShaderBlob->GetBufferSize()
	};

	// ブレンド・ラスタライザの設定
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;

	//ステートの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//depth機能を有効にする
	depthStencilDesc.DepthEnable = true;
	//書き込みをするところ
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はlessEqual。つまり近いと描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	//DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// レンダーターゲットの設定
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// プリミティブトポロジーの設定（三角形として描画）
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// サンプリングの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// PSOの生成
	ComPtr<ID3D12PipelineState> graphicsPipelineState;
	hr = device->CreateGraphicsPipelineState(
		&graphicsPipelineStateDesc,
		IID_PPV_ARGS(graphicsPipelineState.GetAddressOf()));
	assert(SUCCEEDED(hr));

	return graphicsPipelineState;

}


// --- PSO.cpp の一番下に追加 ---

// --- PSO.cpp の CreateComputeRootSignature を修正版に差し替え ---
ComPtr<ID3D12RootSignature> CreateComputeRootSignature(
	ID3D12Device* device,
	HRESULT& hr
) {
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	// [0]: register(b0) 用の32bit定数設定
	// 行列(16個のfloat) + 個数(1個のint) = 計17個の32bit値を直接送れるようにします
	D3D12_ROOT_PARAMETER rootParameter[3] = {};
	rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameter[0].Constants.ShaderRegister = 0; // register(b0)
	rootParameter[0].Constants.Num32BitValues = 17; // 17個の値を送る

	// [1]: register(t0) 用 (gTransforms用のSRV)
	D3D12_DESCRIPTOR_RANGE descriptorRangeSRV{};
	descriptorRangeSRV.BaseShaderRegister = 0; // register(t0)
	descriptorRangeSRV.NumDescriptors = 1;
	descriptorRangeSRV.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeSRV.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameter[1].DescriptorTable.pDescriptorRanges = &descriptorRangeSRV;
	rootParameter[1].DescriptorTable.NumDescriptorRanges = 1;

	// [2]: register(u0) 用 (gInstances用のUAV)
	D3D12_DESCRIPTOR_RANGE descriptorRangeUAV{};
	descriptorRangeUAV.BaseShaderRegister = 0; // register(u0)
	descriptorRangeUAV.NumDescriptors = 1;
	descriptorRangeUAV.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRangeUAV.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameter[2].DescriptorTable.pDescriptorRanges = &descriptorRangeUAV;
	rootParameter[2].DescriptorTable.NumDescriptorRanges = 1;

	descriptionRootSignature.pParameters = rootParameter;
	descriptionRootSignature.NumParameters = _countof(rootParameter);

	// シリアライズと生成（ここはさっきと同じです）
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

	if (FAILED(hr)) {
		Log(logStream, reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	ComPtr<ID3D12RootSignature> rootSignature;
	hr = device->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(rootSignature.GetAddressOf()));
	assert(SUCCEEDED(hr));

	return rootSignature;
}

// ② Compute Shader用のパイプラインステート（PSO）作成
ComPtr<ID3D12PipelineState> CreateComputePipelineState(
	ID3D12Device* device,
	ComPtr<ID3D12RootSignature>& rootSignature,
	HRESULT& hr
) {
	// DXCコンパイラの初期化
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	IDxcIncludeHandler* includeHandler = nullptr;
	DxcCompilerInclude(hr, dxcUtils, dxcCompiler, includeHandler);

	// ★Compute Shader（GpuDrivenObject.CS.hlsl）をコンパイル
	// ターゲットは "cs_6_0" になります
	IDxcBlob* computeShaderBlob = CompileShader(
		L"GpuDrivenObject.CS.hlsl", L"cs_6_0",
		dxcUtils, dxcCompiler, includeHandler);

	// Compute専用のPSO設定（グラフィックス用より設定項目が少なくてシンプル！）
	D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc{};

	rootSignature = CreateComputeRootSignature(device, hr);
	computePsoDesc.pRootSignature = rootSignature.Get();
	computePsoDesc.CS = {
		computeShaderBlob->GetBufferPointer(),
		computeShaderBlob->GetBufferSize()
	};
	computePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	ComPtr<ID3D12PipelineState> computePipelineState;
	hr = device->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(computePipelineState.GetAddressOf()));
	assert(SUCCEEDED(hr));

	return computePipelineState;
}