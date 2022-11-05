#include <array>
#include "common/log.h"
#include "snd_mngr.h"

set_log_channel("snd_mngr")

// 003D3680
static std::array<sndPortInfo, 0x10> PortInfo;
// 003D6040
static std::array<sndCSeSeq, 0x20> SeSequencer;

namespace snd_mngr
{
  // 00373520
  void SInit()
  {
    log_trace("SInit()");

    for (auto& port_info : PortInfo)
    {
      new (&port_info) sndPortInfo;
    }

    for (auto& sequencer : SeSequencer)
    {
      new (&sequencer) sndCSeSeq;
    }
  }
}

// 0018B4B0
void sndTrack::Initialize()
{
  log_trace("sndTrack::Initialize()");

  m_unk_field_0 = 0x7F;
  m_unk_field_1 = 0x7F;
  m_unk_field_2 = 0;
  m_unk_field_3 = 0x40;
  m_unk_field_4 = 0;
  m_unk_field_5 = 0x40;
  m_unk_field_6 = 0;

  m_n_unk_field_C = m_unk_field_C.size();
  for (auto& unk_var : m_unk_field_C)
  {
    unk_var = 0;
  }
}

// 0018C550
void CLoopSeMngr::Initialize()
{
  log_trace("CLoopSeMngr::Initialize()");

  m_n_unk_field_4 = 0;
  m_unk_field_4 = nullptr;
}

// 0018B880
void sndCSeSeq::Initialize()
{
  log_trace("sndCSeSeq::Initialize()");

  m_unk_field_10 = 0;
  m_unk_field_14 = 0;
  m_unk_field_8 = 0;
  m_unk_field_C = 0;
  m_unk_field_18 = 0x7F;
  m_unk_field_28 = 0;
  m_n_unk_field_30 = m_unk_field_30.size();

  for (int i = 0; i < m_n_unk_field_30; ++i)
  {
    m_unk_field_30[i].Initialize();
    m_unk_field_30[i].m_unk_field_6 = i + 0x40;
  }
}

// 00190600
sndCSeSeq::sndCSeSeq()
{
  log_trace("sndCSeSeq::sndCSeSeq()");

  for (int i = 0; i < m_unk_field_30.size(); ++i)
  {
    // FIXME: not sure how this is implemented, 00190620
    for (int i = 0; i < m_unk_field_30[i].m_unk_field_C.size(); i += 4)
    {
      m_unk_field_30[i].m_unk_field_C[i] = 0;
    }

    m_unk_field_30[i].Initialize();
  }

  Initialize();
};

// 00190680
sndPortInfo::sndPortInfo()
{
  log_trace("sndPortInfo::sndPortInfo()");

  // BUG: ? Unintentional?
  // 190680
  for (auto& unk_var : m_unk_field_C)
  {
    unk_var = 0;
  }
  for (auto& unk_var : m_unk_field_21C)
  {
    unk_var.m_unk_field_0 = -1;
  }

  // 001906DC
  m_unk_field_0 = -1;
  m_unk_field_4 = -1;
  m_unk_field_8 = 0;
  m_unk_field_20C = -1;
  m_unk_field_210 = 0;
  m_unk_field_214 = 0;
  m_unk_field_218 = -1;

  for (auto& unk_var : m_unk_field_21C)
  {
    unk_var.m_unk_field_0 = -1;
  }
  for (auto& unk_var : m_unk_field_C)
  {
    unk_var = 0;
  }
}