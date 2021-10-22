/*

 * FrameRateTask

 * Copyright © 2021 Timothy - LiuXuefeng

 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 */

using System;
using System.Threading;
/// <summary>
/// xfggyyds
/// </summary>
namespace Timothy.FrameRateTask
{
	/// <summary>
	/// The class intends to execute a task that need to be executed repeatedly every less than one second and need to be accurate.
	/// </summary>
	/// <typeparam name="TResult">The type of the return value of the task.</typeparam>
	public class FrameRateTaskExecutor<TResult>
	{
		/// <summary>
		/// The actual framerate recently.
		/// </summary>
		public uint FrameRate { get; private set; }

		/// <summary>
		/// Gets a value indicating whether or not the task has finished.
		/// </summary>
		/// <returns>
		/// true if the task has finished; otherwise, false.
		/// </returns>
		public bool Finished { get; private set; } = false;

		/// <summary>
		/// Gets a value indicating whether or not the task has started.
		/// </summary>
		/// <returns>
		/// true if the task has started; otherwise, false.
		/// </returns>
		public bool HasExecuted { get; private set; } = false;
		private readonly object hasExecutedLock = new();
		private bool TrySetExecute()
		{
			lock (hasExecutedLock)
			{
				if (HasExecuted) return false;
				HasExecuted = true;
				return true;
			}
		}

		private TResult result;
		/// <summary>
		/// Get the return value of this task.
		/// </summary>
		/// <exception cref="TaskNotFinishedException">
		///		The task hasn't finished.
		/// </exception>
		public TResult Result
		{
			get
			{
				if (!Finished) throw new TaskNotFinishedException();
				return result;
			}
			private set => result = value;
		}


		/// <summary>
		/// Gets or sets whether it allows time exceeding.
		/// </summary>
		/// <remarks>
		/// If this property is false, the task will throw a TimeExceed exception when the task cannot finish in the given time.
		/// The default value is true.
		/// </remarks>
		public bool AllowTimeExceed { get; set; } = true;

		/// <summary>
		/// It will be called once time exceeds if AllowTimeExceed is set true.
		/// </summary>
		/// <remarks>
		/// parameter bool: If it is called because of the number of time exceeding is greater than MaxTimeExceedCount, the argument is true; if it is called because of exceeding once, the argument is false.
		/// </remarks>
		public Action<bool> TimeExceedAction { get; set; } = callByExceed => { };

		/// <summary>
		/// The TickCount when beginning the loop,
		/// </summary>
		public long BeginTickCount { get; private set; } = 0L;

		/// <summary>
		/// The TickCount should be when ending last loop.
		/// </summary>
		public long LastLoopEndingTickCount { get; private set; }

		/// <summary>
		/// Gets or sets the maximum count of time exceeding continuously.
		/// </summary>
		/// <remarks>
		/// The value is 5 for default.
		/// </remarks>
		public ulong MaxTolerantTimeExceedCount { get; set; } = 5;

		/// <summary>
		/// Start this task synchronously.
		/// </summary>
		/// <exception cref="TaskStartedMoreThanOnceException">
		/// the task has been started.
		/// </exception>
		public void Start()
		{
			if (!TryStart()) throw new TaskStartedMoreThanOnceException();
		}
		/// <summary>
		/// Try to start this task synchronously.
		/// </summary>
		/// <returns>
		/// true if the task is started successfully; false if the task has been started.
		/// </returns>
		public bool TryStart()
		{
			if (!TrySetExecute()) return false;
			loopFunc();
			return true;
		}

		private readonly Action loopFunc;

		/// <summary>
		/// Constructor
		/// </summary>
		/// <param name="loopCondition">If you want to continue to loop, return true; otherwise return false.</param>
		/// <param name="loopToDo">If you want to break out, return false; otherwise, return true.</param>
		/// <param name="timeInterval"></param>
		/// <param name="finallyReturn">The return value. It will be called after the loop.</param>
		/// <param name="maxTotalDuration">The maximum time for the loop to execute.</param>
		public FrameRateTaskExecutor
			(
				Func<bool> loopCondition,
				Func<bool> loopToDo,
				long timeInterval,
				Func<TResult> finallyReturn,
				long maxTotalDuration = long.MaxValue
			)
		{

			if (timeInterval <= 0L && timeInterval > 1000L)
			{
				throw new IllegalTimeIntervalException();
			}
			FrameRate = (uint)(1000L / timeInterval);

			loopFunc = () =>
			{
				ulong timeExceedCount = 0UL;

				var nextTime = (LastLoopEndingTickCount = BeginTickCount = Environment.TickCount64) + timeInterval;
				var endTime = BeginTickCount < long.MaxValue - maxTotalDuration ? BeginTickCount + maxTotalDuration : long.MaxValue;

				uint loopCnt = 0;
				var nextCntTime = BeginTickCount + 1000L;

				while (loopCondition() && nextTime <= endTime)
				{
					if (!loopToDo()) break;

					var nowTime = Environment.TickCount64;
					if (nextTime >= nowTime)
					{
						timeExceedCount = 0UL;
						Thread.Sleep((int)(nextTime - nowTime));
					}
					else
					{
						++timeExceedCount;
						if (timeExceedCount > MaxTolerantTimeExceedCount)
						{
							if (AllowTimeExceed)
							{
								TimeExceedAction(true);
								timeExceedCount = 0UL;
								nextTime = Environment.TickCount64;
							}
							else
							{
								throw new TimeExceedException();
							}
						}
						else if (AllowTimeExceed) TimeExceedAction(false);
					}

					LastLoopEndingTickCount = nextTime;
					nextTime += timeInterval;
					++loopCnt;
					if (Environment.TickCount64 >= nextCntTime)
					{
						nextCntTime = Environment.TickCount64 + 1000L;
						FrameRate = loopCnt;
						loopCnt = 0;
					}
				}

				result = finallyReturn();
				Finished = true;
			};

		}

		/// <summary>
		/// Constructor
		/// </summary>
		/// <param name="loopCondition">If you want to continue to loop, return true; otherwise return false.</param>
		/// <param name="loopToDo">Loop to do.</param>
		/// <param name="timeInterval"></param>
		/// <param name="finallyReturn">The return value. It will be called after the loop.</param>
		/// <param name="maxTotalDuration">The maximum time for the loop to execute.</param>
		public FrameRateTaskExecutor
			(
				Func<bool> loopCondition,
				Action loopToDo,
				long timeInterval,
				Func<TResult> finallyReturn,
				long maxTotalDuration = long.MaxValue
			) : this(loopCondition, () => { loopToDo(); return true; }, timeInterval, finallyReturn, maxTotalDuration) { }
	}
}
