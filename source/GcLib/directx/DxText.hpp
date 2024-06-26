#pragma once

#include "../pch.h"

#include "DxConstant.hpp"
#include "Texture.hpp"
#include "RenderObject.hpp"

namespace directx {
	class DxCharGlyph;
	class DxCharCache;
	class DxCharCacheKey;
	class DxTextRenderer;
	class DxText;

	//*******************************************************************
	//DxFont
	//*******************************************************************
	class DxFont {
		friend DxCharCache;
		friend DxCharCacheKey;
	protected:
		LOGFONT info_;

		D3DCOLOR colorTop_;
		D3DCOLOR colorBottom_;

		TextBorderType typeBorder_;
		LONG widthBorder_;
		D3DCOLOR colorBorder_;
	public:
		DxFont();

		void SetLogFont(LOGFONT& font) { info_ = font; }
		LOGFONT& GetLogFont() { return info_; }
		const LOGFONT& GetLogFont() const { return info_; }

		void SetTopColor(D3DCOLOR color) { colorTop_ = color; }
		const D3DCOLOR GetTopColor() const { return colorTop_; }
		void SetBottomColor(D3DCOLOR color) { colorBottom_ = color; }
		D3DCOLOR GetBottomColor() const { return colorBottom_; }
		
		void SetBorderType(TextBorderType type) { typeBorder_ = type; }
		TextBorderType GetBorderType() const { return typeBorder_; }
		
		void SetBorderWidth(LONG width) { widthBorder_ = width; }
		LONG GetBorderWidth() const { return widthBorder_; }
		
		void SetBorderColor(D3DCOLOR color) { colorBorder_ = color; }
		D3DCOLOR GetBorderColor() const { return colorBorder_; }
	};

	//*******************************************************************
	//DxCharGlyph
	//文字1文字のテクスチャ
	//*******************************************************************
	class DxCharGlyph {
		shared_ptr<Texture> texture_;
		UINT code_;

		GLYPHMETRICS glpMet_;
		POINT size_;
		POINT sizeMax_;
	public:
		DxCharGlyph(UINT code);

		bool Create(gstd::CriticalSection& cs, const gstd::Font& winFont, const DxFont* dxFont);

		shared_ptr<Texture> GetTexture() { return texture_; }

		const POINT& GetSize() const { return size_; }
		const POINT& GetMaxSize() const { return sizeMax_; }
		const GLYPHMETRICS* GetGM() const { return &glpMet_; }
	};


	//*******************************************************************
	//DxCharCache
	//文字キャッシュ
	//*******************************************************************
	class DxCharCacheKey {
		friend DxCharCache;
		friend DxTextRenderer;
	private:
		UINT code_;
		DxFont font_;
	public:
		bool operator ==(const DxCharCacheKey& key) const {
			bool res = true;
			res &= (code_ == key.code_);
			res &= (font_.colorTop_ == key.font_.colorTop_);
			res &= (font_.colorBottom_ == key.font_.colorBottom_);
			res &= (font_.typeBorder_ == key.font_.typeBorder_);
			res &= (font_.widthBorder_ == key.font_.widthBorder_);
			res &= (font_.colorBorder_ == key.font_.colorBorder_);
			if (!res) return res;
			res &= (memcmp(&key.font_.info_, &font_.info_, sizeof(LOGFONT)) == 0);
			return res;
		}
		bool operator<(const DxCharCacheKey& key) const {
			if (code_ != key.code_) return code_ < key.code_;
			if (font_.colorTop_ != key.font_.colorTop_) return font_.colorTop_ < key.font_.colorTop_;
			if (font_.colorBottom_ != key.font_.colorBottom_) return font_.colorBottom_ < key.font_.colorBottom_;
			if (font_.typeBorder_ != key.font_.typeBorder_) return (font_.typeBorder_ < key.font_.typeBorder_ );
			if (font_.widthBorder_ != key.font_.widthBorder_) return font_.widthBorder_ < key.font_.widthBorder_;
			if (font_.colorBorder_ != key.font_.colorBorder_) return font_.colorBorder_ < key.font_.colorBorder_;
			return (memcmp(&key.font_.info_, &font_.info_, sizeof(LOGFONT)) < 0);
		}
	};
	class DxCharCache {
		friend DxTextRenderer;
	public:
		enum : size_t {
			MAX = 1U << 14,
		};
	private:
		int countPri_;
		std::map<DxCharCacheKey, DxCharGlyph> mapCache_;
		std::map<int, DxCharCacheKey> mapPriKey_;
		std::map<DxCharCacheKey, int> mapKeyPri_;

		void _arrange();
	public:
		DxCharCache();
		~DxCharCache();

		void Clear();
		size_t GetCacheCount() const { return mapCache_.size(); }

		DxCharGlyph* GetChar(const DxCharCacheKey& key);
		DxCharGlyph* AddChar(const DxCharCacheKey& key, DxCharGlyph&& value);
	};

	//*******************************************************************
	//DxTextScanner
	//*******************************************************************
	class DxTextScanner;
	class DxTextToken {
		friend DxTextScanner;
	public:
		enum class Type : uint8_t {
			Unknown, Eof, Newline,

			Identifier,
			Int, Hex, Real, String,

			OpenP, CloseP, OpenB, CloseB, OpenC, CloseC,
			Sharp,

			Comma, Equal,
			Colon, Semicolon,

			Text,
		};
	protected:
		Type type_;
		std::wstring element_;
	public:
		DxTextToken() { type_ = Type::Unknown; }
		DxTextToken(Type type, const std::wstring& element) { type_ = type; element_ = element; }
		virtual ~DxTextToken() {}

		Type GetType() { return type_; }
		std::wstring& GetElement() { return element_; }

		int64_t GetInteger();
		double GetReal();
		bool GetBoolean();
		std::wstring GetString();
		std::wstring& GetIdentifier();
	};

	class DxTextScanner {
	public:
		using TkType = DxTextToken::Type;
	protected:
		int line_;
		std::vector<wchar_t> buffer_;
		std::vector<wchar_t>::iterator pointer_;
		std::vector<wchar_t>::iterator end_;
		DxTextToken token_;
		bool bTagScan_;

		bool _CheckEnd();
		wchar_t _NextChar();
		wchar_t _PeekNextChar(int index);

		virtual void _SkipSpace();
		virtual void _RaiseError(const std::wstring& str);

		bool _IsTextStartSign();
		bool _IsTextScan();
	public:
		DxTextScanner(wchar_t* str, size_t charCount);
		DxTextScanner(const std::wstring& str);
		DxTextScanner(std::vector<wchar_t>& buf);
		virtual ~DxTextScanner();

		DxTextToken& GetToken();
		DxTextToken& Next();

		bool HasNext();
		void CheckType(DxTextToken& tok, TkType type);
		void CheckIdentifer(DxTextToken& tok, const std::wstring& id);
		int GetCurrentLine();
		int SearchCurrentLine();

		std::vector<wchar_t>::iterator GetCurrentPointer();
		void SetCurrentPointer(std::vector<wchar_t>::iterator pos);
		void SetPointerBegin() { pointer_ = buffer_.begin(); }
		int GetCurrentPosition();
		void SetTagScanEnable(bool bEnable) { bTagScan_ = bEnable; }
	};

	//*******************************************************************
	//DxTextRenderer
	//テキスト描画エンジン
	//*******************************************************************
	class DxTextLine;
	class DxTextInfo;
	class DxTextRenderer;

	class DxTextTag {
	protected:
		TextTagType typeTag_;
		int indexTag_;
	public:
		DxTextTag(TextTagType typeTag);
		virtual ~DxTextTag() {}

		virtual DxTextTag* Clone() const = 0;

		TextTagType GetTagType() const { return typeTag_; }
		int GetTagIndex() const { return indexTag_; }
		void SetTagIndex(int index) { indexTag_ = index; }
	};
	class DxTextTag_Ruby : public DxTextTag {
		LONG leftMargin_;
		LONG topMargin_;

		std::wstring text_;
		std::wstring ruby_;

		shared_ptr<DxText> dxText_;
	public:
		DxTextTag_Ruby();

		virtual DxTextTag_Ruby* Clone() const override { return new DxTextTag_Ruby(*this); }
		
		LONG GetLeftMargin() const { return leftMargin_; }
		void SetLeftMargin(LONG left) { leftMargin_ = left; }

		LONG GetTopMargin() const { return topMargin_; }
		void SetTopMargin(LONG top) { topMargin_ = top; }

		const std::wstring& GetText() const { return text_; }
		void SetText(const std::wstring& text) { text_ = text; }
		const std::wstring& GetRuby() const { return ruby_; }
		void SetRuby(const std::wstring& ruby) { ruby_ = ruby; }

		shared_ptr<DxText> GetRenderText() { return dxText_; }
		void SetRenderText(shared_ptr<DxText> text) { dxText_ = text; }
	};
	class DxTextTag_Font : public DxTextTag {
		DxFont font_;
		D3DXVECTOR2 offset_;
	public:
		DxTextTag_Font();

		virtual DxTextTag_Font* Clone() const override { return new DxTextTag_Font(*this); }
		
		void SetFont(DxFont& font) { font_ = font; }
		const DxFont& GetFont() const { return font_; }

		void SetOffset(D3DXVECTOR2& off) { offset_ = off; }
		D3DXVECTOR2& GetOffset() { return offset_; }
		const D3DXVECTOR2& GetOffset() const { return offset_; }
	};

	class DxTextLine {
		friend DxTextRenderer;
	protected:
		LONG width_;
		LONG height_;
		LONG sidePitch_;

		std::vector<UINT> code_;

		std::vector<unique_ptr<DxTextTag>> tag_;
	public:
		DxTextLine();
		DxTextLine(const DxTextLine& other);

		DxTextLine& operator=(DxTextLine other) noexcept;
		void swap(DxTextLine& other) noexcept;
		
		LONG GetWidth() const { return width_; }
		LONG GetHeight() const { return height_; }
		LONG GetSidePitch() const { return sidePitch_; }
		void SetSidePitch(LONG pitch) { sidePitch_ = pitch; }

		const std::vector<UINT>& GetTextCodes() const { return code_; }
		size_t GetTextCodeCount() const { return code_.size(); }

		size_t GetTagCount() const { return tag_.size(); }
		const DxTextTag* GetTag(size_t index) const { return tag_[index].get(); }
	};

	class DxTextInfo {
		friend DxTextRenderer;
	protected:
		LONG totalWidth_;
		LONG totalHeight_;

		int lineValidStart_;
		int lineValidEnd_;
		bool bAutoIndent_;

		std::vector<DxTextLine> textLine_;
	public:
		DxTextInfo();

		LONG GetTotalWidth() const { return totalWidth_; }
		LONG GetTotalHeight() const { return totalHeight_; }

		int GetValidStartLine() const { return lineValidStart_; }
		int GetValidEndLine() const { return lineValidEnd_; }
		void SetValidStartLine(int line) { lineValidStart_ = line; }
		void SetValidEndLine(int line) { lineValidEnd_ = line; }
		bool IsAutoIndent() const { return bAutoIndent_; }
		void SetAutoIndent(bool bEnable) { bAutoIndent_ = bEnable; }

		size_t GetLineCount() const { return textLine_.size(); }
		void AddTextLine(DxTextLine&& text) { textLine_.push_back(text), lineValidEnd_++; }
		const DxTextLine& GetTextLine(size_t pos) const { return textLine_[pos]; }
	};

	class DxTextRenderObject {
		struct ObjectData {
			POINT bias;
			shared_ptr<Sprite2D> sprite;
		};
	protected:
		POINT position_;//移動先座標
		D3DXVECTOR3 scale_;//拡大率
		D3DXVECTOR3 angle_;
		D3DCOLOR color_;
		std::list<ObjectData> listData_;
		D3DXVECTOR2 center_;//座標変換の中心
		bool bAutoCenter_;
		bool bPermitCamera_;
		shared_ptr<Shader> shader_;
	public:
		DxTextRenderObject();
		virtual ~DxTextRenderObject();

		void Render();
		void Render(const D3DXVECTOR2& angleX, const D3DXVECTOR2& angleY, const D3DXVECTOR2& angleZ);
		void AddRenderObject(shared_ptr<Sprite2D> obj);
		void AddRenderObject(shared_ptr<DxTextRenderObject> obj, const POINT& bias);

		POINT& GetPosition() { return position_; }
		void SetPosition(const POINT& pos) { position_.x = pos.x; position_.y = pos.y; }
		void SetPosition(LONG x, LONG y) { position_.x = x; position_.y = y; }
		void SetVertexColor(D3DCOLOR color) { color_ = color; }

		void SetAngle(const D3DXVECTOR3& angle) { angle_ = angle; }
		void SetScale(const D3DXVECTOR3& scale) { scale_ = scale; }
		void SetTransCenter(const D3DXVECTOR2& center) { center_ = center; }
		void SetAutoCenter(bool bAuto) { bAutoCenter_ = bAuto; }
		void SetPermitCamera(bool bPermit) { bPermitCamera_ = bPermit; }

		shared_ptr<Shader> GetShader() { return shader_; }
		void SetShader(shared_ptr<Shader> shader) { shader_ = shader; }
	};

	class DxTextRenderer {
		static DxTextRenderer* thisBase_;
	protected:
		DxCharCache cache_;
		gstd::Font winFont_;
		D3DCOLOR colorVertex_;
		gstd::CriticalSection lock_;

		SIZE _GetTextSize(HDC hDC, wchar_t* pText);

		bool _GetTextInfoSub(DxTextLine& destLine,
			const std::wstring& text, DxText* dxText, DxTextInfo* textInfo,
			HDC& hDC, LONG& totalWidth, LONG& totalHeight);

		void _CreateRenderObject(shared_ptr<DxTextRenderObject> objRender, DxText* pDxText, 
			const POINT& pos, DxFont dxFont, const DxTextLine& textLine);

		std::wstring _ReplaceRenderText(std::wstring text);
	public:
		DxTextRenderer();
		virtual ~DxTextRenderer();

		static DxTextRenderer* GetBase() { return thisBase_; }
		gstd::CriticalSection& GetLock() { return lock_; }

		bool Initialize();

		void ClearCache() { cache_.Clear(); }
		void SetFont(LOGFONT& logFont);
		void SetVertexColor(D3DCOLOR color) { colorVertex_ = color; }

		DxTextInfo CreateTextInfo(DxText* dxText);

		shared_ptr<DxTextRenderObject> CreateRenderObject(DxText* dxText, const DxTextInfo& textInfo);

		void Render(DxText* dxText);
		void Render(DxText* dxText, const DxTextInfo& textInfo);

		size_t GetCacheCount() { return cache_.GetCacheCount(); }

		bool AddFontFromFile(const std::wstring& path);
	};

	//*******************************************************************
	//DxText
	//テキスト描画
	//*******************************************************************
	class DxText {
		friend DxTextRenderer;
	protected:
		DxFont dxFont_;

		POINT pos_;
		LONG widthMax_;
		LONG heightMax_;
		float sidePitch_;
		float linePitch_;
		float fixedWidth_;
		DxRect<LONG> margin_;
		TextAlignment alignmentHorizontal_;
		TextAlignment alignmentVertical_;
		D3DCOLOR colorVertex_;
		bool bPermitCamera_;
		bool bSyntacticAnalysis_;

		shared_ptr<Shader> shader_;
		std::wstring text_;
		size_t textHash_;
	public:
		DxText();
		virtual ~DxText();

		void Copy(const DxText& src);
		virtual void Render();
		void Render(const DxTextInfo& textInfo);

		DxTextInfo CreateTextInfo();
		shared_ptr<DxTextRenderObject> CreateRenderObject();
		shared_ptr<DxTextRenderObject> CreateRenderObject(const DxTextInfo& textInfo);

		DxFont& GetFont() { return dxFont_; }
		void SetFont(DxFont& font) { dxFont_ = font; }
		void SetFont(LOGFONT& logFont) { dxFont_.SetLogFont(logFont); }

		void SetFontType(const wchar_t* type);
		LONG GetFontSize() { return dxFont_.GetLogFont().lfHeight; }
		void SetFontSize(LONG size) { dxFont_.GetLogFont().lfHeight = size; }
		LONG GetFontWeight() { return dxFont_.GetLogFont().lfWeight; }
		void SetFontWeight(LONG weight) { dxFont_.GetLogFont().lfWeight = weight; }
		bool GetFontItalic() { return dxFont_.GetLogFont().lfItalic; }
		void SetFontItalic(bool bItalic) { dxFont_.GetLogFont().lfItalic = bItalic; }
		bool GetFontUnderLine() { return dxFont_.GetLogFont().lfUnderline; }
		void SetFontUnderLine(bool bLine) { dxFont_.GetLogFont().lfUnderline = bLine; }

		void SetFontCharset(BYTE set) { dxFont_.GetLogFont().lfCharSet = set; }

		void SetFontColorTop(D3DCOLOR color) { dxFont_.SetTopColor(color); }
		void SetFontColorBottom(D3DCOLOR color) { dxFont_.SetBottomColor(color); }
		void SetFontBorderWidth(LONG width) { dxFont_.SetBorderWidth(width); }
		void SetFontBorderType(TextBorderType type) { dxFont_.SetBorderType(type); }
		void SetFontBorderColor(D3DCOLOR color) { dxFont_.SetBorderColor(color); }

		POINT& GetPosition() { return pos_; }
		void SetPosition(LONG x, LONG y) { pos_.x = x; pos_.y = y; }
		void SetPosition(const POINT& pos) { pos_ = pos; }
		LONG GetMaxWidth() { return widthMax_; }
		void SetMaxWidth(LONG width) { widthMax_ = width; }
		LONG GetMaxHeight() { return heightMax_; }
		void SetMaxHeight(LONG height) { heightMax_ = height; }
		float GetSidePitch() { return sidePitch_; }
		void SetSidePitch(float pitch) { sidePitch_ = pitch; }
		float GetLinePitch() { return linePitch_; }
		void SetLinePitch(float pitch) { linePitch_ = pitch; }
		float GetFixedWidth() { return fixedWidth_; }
		void SetFixedWidth(float width) { fixedWidth_ = width; }
		DxRect<LONG>& GetMargin() { return margin_; }
		void SetMargin(DxRect<LONG>& margin) { margin_ = margin; }
		TextAlignment GetHorizontalAlignment() { return alignmentHorizontal_; }
		void SetHorizontalAlignment(TextAlignment value) { alignmentHorizontal_ = value; }
		TextAlignment GetVerticalAlignment() { return alignmentVertical_; }
		void SetVerticalAlignment(TextAlignment value) { alignmentVertical_ = value; }

		D3DCOLOR GetVertexColor() { return colorVertex_; }
		void SetVertexColor(D3DCOLOR color) { colorVertex_ = color; }
		bool IsPermitCamera() { return bPermitCamera_; }
		void SetPermitCamera(bool bPermit) { bPermitCamera_ = bPermit; }
		bool IsSyntacticAnalysis() { return bSyntacticAnalysis_; }
		void SetSyntacticAnalysis(bool bEnable) { bSyntacticAnalysis_ = bEnable; }

		std::wstring& GetText() { return text_; }
		void SetText(const std::wstring& text) { text_ = text; }
		void SetTextHash(size_t hash) { textHash_ = hash; }
		size_t GetTextHash() { return textHash_; }

		shared_ptr<Shader> GetShader() { return shader_; }
		void SetShader(shared_ptr<Shader> shader) { shader_ = shader; }
	};
}