export module md_span;

export namespace ABL
{
	//TODO: replace with c++23 mdspan
	template<typename DataType>
	struct md_span
	{
	private:
		DataType* Data;
		int UnderlyingWidth = 0;
		int UnderlyingHeight = 0;

		int Width = 0;
		int Height = 0;

		int BeginX = 0;
		int BeginY = 0;
		int StartIndex = 0;

		int EndX = 0;
		int EndY = 0;
		int EndIndex = 0;

	public:
		struct iterator
		{
		private:
			DataType* Pointer;

			int X = 0;
			int MinX = 0;
			int MaxX = 0;
			int TotalWidth = 0;

		public:
			constexpr iterator(DataType* InPointer, int InX, int InMinX, int InMaxX, int InTotalWidth)
				: Pointer{ InPointer }, X{ InX }, MinX{ InMinX }, MaxX{ InMaxX }, TotalWidth{ InTotalWidth }
			{}

			constexpr DataType& operator*() const { return *Pointer; }
			constexpr DataType* operator->() { return Pointer; }

// 			constexpr iterator& operator++()
// 			{
// 				//TODO: clean this up.
// 				int StepAmount = 1;
// 				if (X + StepAmount <= MaxX)
// 				{
// 					++X;
// 				}
// 				else
// 				{
// 					X = MinX;
// 					StepAmount = TotalWidth - MaxX + MinX + 1;
// 				}
// 
// 				Pointer += StepAmount;
// 				return *this;
// 			}

			constexpr iterator& operator++()
			{
				auto StepAmount = X + 1 <= MaxX ? 1 : TotalWidth - MaxX + MinX + 1;
				X += StepAmount;
				X %= TotalWidth;
				Pointer += StepAmount;
				return *this;
			}

			constexpr friend bool operator==(const iterator& Left, const iterator& Right) { return Left.Pointer == Right.Pointer; }
			constexpr friend bool operator!=(const iterator& Left, const iterator& Right) { return Left.Pointer != Right.Pointer; }
		};

		constexpr md_span() {}

		constexpr md_span(DataType* InData, int InUnderlyingWidth, int InUnderlyingHeight
			, int InWidth, int InHeight, int InBeginX = 0, int InBeginY = 0)
			: Data{ InData }, UnderlyingWidth{ InUnderlyingWidth }, UnderlyingHeight{ InUnderlyingHeight }
			, Width{ InWidth }, Height{ InHeight }, BeginX{ InBeginX }, BeginY{ InBeginY }
		{
			StartIndex = BeginX + BeginY * UnderlyingWidth;
			EndX = BeginX + Width;
			EndY = BeginY + Height - 1;
			EndIndex = EndX + EndY * UnderlyingWidth;
		}

		constexpr md_span subspan(int InWidth, int InHeight, int InBeginX, int InBeginY)
		{
			return
			{
				Data, UnderlyingWidth, UnderlyingHeight, InWidth, InHeight, InBeginX, InBeginY
			};
		}

		constexpr iterator begin() { return iterator{ &Data[StartIndex], BeginX, BeginX, EndX, UnderlyingWidth }; }
		constexpr iterator end() { return iterator{ &Data[EndIndex], EndX, BeginX, EndX, UnderlyingWidth }; }
	};
}
