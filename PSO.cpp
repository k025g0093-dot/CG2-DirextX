#include "PSO.h"

// ルートシグネチャの生成
// シェーダーにどんなリソースを渡すかを定義する
ID3D12RootSignature* CreateRootSignature(
	ID3D12Device* device,
	HRESULT& hr) {

	// ルートシグネチャの設定
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;//0から始まる
	descriptorRange[0].NumDescriptors = 1;//数は一つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//自動で決める].OffsetInDescriptorsFromTableStart


	// ルートパラメータの設定（シェーダーに渡すリソースの種類を定義）
	D3D12_ROOT_PARAMETER rootParameter[3] = {};
	rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // 定数バッファビューを使用
	rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // 全シェーダーから参照可能
	rootParameter[0].Descriptor.ShaderRegister = 0; // register(b0)に対応
	
	rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // シェーダーリソースビューを使用
	rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // 全シェーダーから参照可能
	rootParameter[1].Descriptor.ShaderRegister = 0; // register(b0)に対応

	rootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // 定数バッファビューを使用
	rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameter[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	descriptionRootSignature.pParameters = rootParameter;
	descriptionRootSignature.NumParameters = _countof(rootParameter);


	//Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1]={};
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
	ID3D12RootSignature* rootSignature = nullptr;
	hr = device->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	return rootSignature;
}

// インプットレイアウトの生成
// 頂点データの形式をGPUに教える
D3D12_INPUT_LAYOUT_DESC CreateLayout() {

	// 頂点データの要素定義（今回はPOSITIONのみ）
	static D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION"; // シェーダー側のセマンティクス名
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // float4形式
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD"; // シェーダー側のセマンティクス名
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT; // float2形式
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

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
ID3D12PipelineState* CreatePipelineStateDesc(
	ID3D12Device* device,
	ID3D12RootSignature*& rootSignature,
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
	graphicsPipelineStateDesc.pRootSignature = rootSignature;

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
	static ID3D12PipelineState* graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(
		&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	return graphicsPipelineState;
}