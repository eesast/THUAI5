/*

 * FrameRateTask

 * Copyright © 2021 Timothy - LiuXuefeng

 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 */

using System;

namespace Timothy.FrameRateTask
{
	/// <summary>
	/// This exception will be throwed when the task hasn't finished but is asked to get the return value.
	/// </summary>
	public class TaskNotFinishedException : Exception
	{
		///
		public override string Message => "The task has not finished!";
	}

	/// <summary>
	/// This exception will be throwed when the time interval specified is invalid.
	/// </summary>
	public class IllegalTimeIntervalException : Exception
	{
		///
		public override string Message => "The time interval should be positive and no more than 1000ms!";
	}

	/// <summary>
	/// This exception will be throwed when time exceeds but time exceeding is not allowed.
	/// </summary>
	public class TimeExceedException : Exception
	{
		///
		public override string Message => "The loop runs too slow that it cannot finish the task in the given time!";
	}

	/// <summary>
	/// This exception will be throwed when the task has been started but is asked to be started again.
	/// </summary>
	public class TaskStartedMoreThanOnceException : Exception
	{
		///
		public override string Message => "The task has been started more than once!";
	}

}
