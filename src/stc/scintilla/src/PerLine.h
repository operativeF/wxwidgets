// Scintilla source code edit control
/** @file PerLine.h
 ** Manages data associated with each line of the document
 **/
// Copyright 1998-2009 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef PERLINE_H
#define PERLINE_H

#ifdef SCI_NAMESPACE
namespace Scintilla {
#endif

/**
 * This holds the marker identifier and the marker type to display.
 * MarkerHandleNumbers are members of lists.
 */
struct MarkerHandleNumber {
	int handle;
	int number;
	MarkerHandleNumber *next;
};

/**
 * A marker handle set contains any number of MarkerHandleNumbers.
 */
class MarkerHandleSet {
	MarkerHandleNumber *root;

public:
	MarkerHandleSet();
	~MarkerHandleSet();
	int Length() const;
	int MarkValue() const;	///< Bit set of marker numbers.
	bool Contains(int handle) const;
	bool InsertHandle(int handle, int markerNum);
	void RemoveHandle(int handle);
	bool RemoveNumber(int markerNum, bool all);
	void CombineWith(MarkerHandleSet *other);
};

class LineMarkers : public PerLine {
	SplitVector<MarkerHandleSet *> markers;
	/// Handles are allocated sequentially and should never have to be reused as 32 bit ints are very big.
	int handleCurrent{0};
public:
	LineMarkers()  = default;
	~LineMarkers() override;
	void Init() override;
	void InsertLine(int line) override;
	void RemoveLine(int line) override;

	int MarkValue(int line);
	int MarkerNext(int lineStart, int mask) const;
	int AddMark(int line, int marker, int lines);
	void MergeMarkers(int pos);
	bool DeleteMark(int line, int markerNum, bool all);
	void DeleteMarkFromHandle(int markerHandle);
	int LineFromHandle(int markerHandle);
};

class LineLevels : public PerLine {
	SplitVector<int> levels;
public:
	~LineLevels() override;
	void Init() override;
	void InsertLine(int line) override;
	void RemoveLine(int line) override;

	void ExpandLevels(int sizeNew=-1);
	void ClearLevels();
	int SetLevel(int line, int level, int lines);
	int GetLevel(int line) const;
};

class LineState : public PerLine {
	SplitVector<int> lineStates;
public:
	LineState() = default;
	~LineState() override;
	void Init() override;
	void InsertLine(int line) override;
	void RemoveLine(int line) override;

	int SetLineState(int line, int state);
	int GetLineState(int line);
	int GetMaxLineState() const;
};

class LineAnnotation : public PerLine {
	SplitVector<char *> annotations;
public:
	LineAnnotation() = default;
	~LineAnnotation() override;
	void Init() override;
	void InsertLine(int line) override;
	void RemoveLine(int line) override;

	bool MultipleStyles(int line) const;
	int Style(int line) const;
	const char *Text(int line) const;
	const unsigned char *Styles(int line) const;
	void SetText(int line, const char *text);
	void ClearAll();
	void SetStyle(int line, int style);
	void SetStyles(int line, const unsigned char *styles);
	int Length(int line) const;
	int Lines(int line) const;
};

using TabstopList = std::vector<int>;

class LineTabstops : public PerLine {
	SplitVector<TabstopList *> tabstops;
public:
	LineTabstops() = default;
	~LineTabstops() override;
	void Init() override;
	void InsertLine(int line) override;
	void RemoveLine(int line) override;

	bool ClearTabstops(int line);
	bool AddTabstop(int line, int x);
	int GetNextTabstop(int line, int x) const;
};

#ifdef SCI_NAMESPACE
}
#endif

#endif
