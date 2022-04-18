// Copyright 2018 The Beam Team
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "../utility/containers.h"
#include "../core/block_crypt.h"
#include "../utility/io/timer.h"

namespace beam {

struct TxPool
{
	struct Stats
	{
		Amount m_Fee;
		Amount m_FeeReserve;
		uint32_t m_Size;
		uint32_t m_SizeCorrection;
		HeightRange m_Hr;

		void From(const Transaction&, const Transaction::Context&, Amount feeReserve, uint32_t nSizeCorrection);
		void SetSize(const Transaction&);
	};

	struct Profit
		:public boost::intrusive::set_base_hook<>
	{
		Stats m_Stats;

		bool operator < (const Profit& t) const;
	};

	struct Fluff
	{
		enum State {
			PreFluffed,
			Fluffed,
			Outdated,
		};

		struct Element
		{
			Transaction::Ptr m_pValue;
			State m_State;

			struct Tx
				:public intrusive::set_base_hook<Transaction::KeyType>
			{
				IMPLEMENT_GET_PARENT_OBJ(Element, m_Tx)
			} m_Tx;

			struct Profit
				:public TxPool::Profit
			{
				IMPLEMENT_GET_PARENT_OBJ(Element, m_Profit)
			} m_Profit;

			struct Hist
				:public boost::intrusive::list_base_hook<>
			{
				Height m_Height;
				IMPLEMENT_GET_PARENT_OBJ(Element, m_Hist)
			} m_Hist;

			struct Send
				:public boost::intrusive::list_base_hook<>
			{
				Element* m_pThis;
				uint32_t m_Refs = 0;
			};
			Send* m_pSend = nullptr;
		};

		typedef boost::intrusive::multiset<Element::Tx> TxSet;
		typedef boost::intrusive::multiset<Element::Profit> ProfitSet;
		typedef boost::intrusive::list<Element::Hist> HistList;
		typedef boost::intrusive::list<Element::Send> SendQueue;

		TxSet m_setTxs;
		ProfitSet m_setProfit;
		SendQueue m_SendQueue;
		HistList m_lstOutdated;
		HistList m_lstWaitFluff;

		Element* AddValidTx(Transaction::Ptr&&, const Stats&, const Transaction::KeyType&, State, Height hLst = 0);
		void SetState(Element&, State);
		void Delete(Element&);
		void Release(Element::Send&);
		void Clear();

		~Fluff() { Clear(); }

	private:

		struct Features
		{
			bool m_SendAndProfit;
			bool m_TxSet;
			bool m_WaitFluff;
			bool m_Outdated;

			static Features get(State);
		};

		void SetState(Element&, Features f0, Features f);
		static void SetStateHistIn(Element&, HistList&, bool b0, bool b);
		static void SetStateHistOut(Element&, HistList&, bool b0, bool b);

	};

	struct Stem
	{
		struct Element
		{
			Transaction::Ptr m_pValue;

			struct Time
				:public boost::intrusive::set_base_hook<>
			{
				uint32_t m_Value;

				bool operator < (const Time& t) const { return m_Value < t.m_Value; }

				IMPLEMENT_GET_PARENT_OBJ(Element, m_Time)
			} m_Time;

			struct Profit
				:public TxPool::Profit
			{
				IMPLEMENT_GET_PARENT_OBJ(Element, m_Profit)
			} m_Profit;

			Stats m_Stats;
		};

		typedef boost::intrusive::multiset<Element::Time> TimeSet;
		typedef boost::intrusive::multiset<Element::Profit> ProfitSet;

		TimeSet m_setTime;
		ProfitSet m_setProfit;

		void Delete(Element&);
		void Clear();
		void InsertAggr(Element&);
		void DeleteAggr(Element&);
		void DeleteTimer(Element&);

		bool TryMerge(Element& trg, Element& src);

		Element* get_NextTimeout(uint32_t& nTimeout_ms);
		void SetTimer(uint32_t nTimeout_ms, Element&);
		void KillTimer();

		io::Timer::Ptr m_pTimer; // set during the 1st phase
		void OnTimer();

		~Stem() { Clear(); }

		virtual bool ValidateTxContext(const Transaction&, const HeightRange&, const AmountBig::Type& fees, Amount& feeReserve) = 0; // assuming context-free validation is already performed, but 
		virtual void OnTimedOut(Element&) = 0;

	private:
		void DeleteRaw(Element&);
		void SetTimerRaw(uint32_t nTimeout_ms);
	};

	struct Dependent
	{
		struct Element
		{
			Transaction::Ptr m_pValue;
			Element* m_pParent;

			// cumulative
			Amount m_Fee;
			uint32_t m_BvmCharge;
			uint32_t m_Size;
			uint32_t m_Depth;

			struct Tx
				:public intrusive::set_base_hook<Transaction::KeyType>
			{
				IMPLEMENT_GET_PARENT_OBJ(Element, m_Tx)
			} m_Tx;

			struct Context
				:public intrusive::set_base_hook<Merkle::Hash>
			{
				IMPLEMENT_GET_PARENT_OBJ(Element, m_Context)
			} m_Context;

			bool m_Fluff = false;
		};

		typedef boost::intrusive::multiset<Element::Tx> TxSet;
		typedef boost::intrusive::multiset<Element::Context> ContextSet;

		TxSet m_setTxs;
		ContextSet m_setContexts;

		Element* m_pBest;

		Element* AddValidTx(Transaction::Ptr&&, const Transaction::Context&, const Transaction::KeyType& key, const Merkle::Hash&, Element* pParent);
		void Clear();

		Dependent() :m_pBest(nullptr) {}

		~Dependent() { Clear(); }

	private:
		bool ShouldUpdateBest(const Element&);
	};


};


} // namespace beam
