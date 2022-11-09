#include "common/debug.h"
#include "common/log.h"
#include "common/types.h"
#include "mg_memory.h"
#include "mg_tanime.h"
#include "mg_texture.h"
#include "script_interpreter.h"

set_log_channel("mg_texture");

// 00376C1C
static mgCTextureAnime* pTexAnime{ nullptr };
// 00376C20
static mgCTextureAnime* pLoadTexAnime{ nullptr };
// 00376C24
static s32 now_group{ 0 };
// 00376C28
static mgCTextureManager* TexManager{ nullptr };
// 00376C2C
static mgCMemory* TexAnimeStack{ nullptr };
// 00376C30
static char* group_name{ nullptr };
// 00376C34
static int ta_enable{ 0 };
// 00376C38
static usize now_texb{ 0 };
// 00376C3C
static bool texBugPatch{ false };

static bool texTEX_ANIME(SPI_STACK* stack, int)
{
  const char* s = spiGetStackString(stack);

  if (s)
  {
    auto len = strlen(s) + 1;
    auto dest = static_cast<char*>(TexAnimeStack->Alloc((len >> 4) + 1));

    strcpy(dest, s);

    group_name = dest;
  }

  ta_enable = spiGetStackInt(++stack);

  return true;
}

static bool texTEX_ANIME_DATA(SPI_STACK* stack, int)
{
  todo;

  return true;
}

static bool texSRC_TEX(SPI_STACK* stack, int)
{
  todo;

  return true;
}

static bool texDEST_TEX(SPI_STACK* stack, int)
{
  todo;

  return true;
}

static bool texSCROLL(SPI_STACK* stack, int)
{
  todo;

  return true;
}

static bool texCLUT_COPY(SPI_STACK* stack, int)
{
  todo;

  return true;
}

static bool texCOLOR(SPI_STACK* stack, int)
{
  todo;

  return true;
}

static bool texALPHA_BLEND(SPI_STACK* stack, int)
{
  todo;

  return true;
}

static bool texALPHA_TEST(SPI_STACK* stack, int)
{
  todo;

  return true;
}

static bool texWAIT(SPI_STACK* stack, int)
{
  todo;

  return true;
}

static bool texTEX_ANIME_DATA_END(SPI_STACK* stack, int)
{
  todo;

  return true;
}

static bool texANIME_END(SPI_STACK* stack, int)
{
  todo;

  return true;
}

static bool texANIME_BUG_PATCH(SPI_STACK* stack, int)
{
  todo;

  return true;
}

// 003340b0
const std::array<SPI_TAG_PARAM, 14> tex_tag =
{
  "TEX_ANIME",          texTEX_ANIME,
  "TEX_ANIME_DATA",     texTEX_ANIME_DATA,
  "SRC_TEX",            texSRC_TEX,
  "DEST_TEX",           texDEST_TEX,
  "SCROLL",             texSCROLL,
  "CLUT_COPY",          texCLUT_COPY,
  "COLOR",              texCOLOR,
  "ALPHA_BLEND",        texALPHA_BLEND,
  "ALPHA_TEST",         texALPHA_TEST,
  "WAIT",               texWAIT,
  "TEX_ANIME_DATA_END", texTEX_ANIME_DATA_END,
  "TEX_ANIME_END",      texANIME_END,
  "BUG_PATCH",          texANIME_BUG_PATCH,
  NULL, nullptr
};

// 0012C6D0
mgCTextureBlock::mgCTextureBlock()
{
  log_trace("mgCTextureBlock::mgCTextureBlock()");

  Initialize();
}

// 0012C700
void mgCTextureBlock::Initialize()
{
  log_trace("mgCTextureBlock::Initialize()");

  m_unk_field_0 = nullptr;
  m_unk_field_4 = nullptr;
  m_texture = nullptr;
  m_texture_anime = nullptr;
}

// 0012C720
void mgCTextureBlock::Add(mgCTexture* texture)
{
  log_trace("mgCTextureBlock::Add({})", fmt::ptr(texture));

  texture->m_next = nullptr;
  if (m_texture == nullptr)
  {
    m_texture = texture;
    return;
  }

  mgCTexture* tex = m_texture;
  for (; tex->m_next != nullptr; tex = tex->m_next) {}
  tex->m_next = texture;
}

// 0012C780
void mgCTextureBlock::Delete(mgCTexture* texture)
{
  log_trace("mgCTextureBlock::Delete({})", fmt::ptr(texture));

  mgCTexture* prev_tex = nullptr;
  for (mgCTexture* tex = m_texture; tex->m_next != nullptr; tex = tex->m_next)
  {
    if (texture == tex)
    {
      if (prev_tex == nullptr)
      {
        m_texture = tex->m_next;
      }
      else
      {
        prev_tex->m_next = tex->m_next;
      }
    }
    prev_tex = tex;
  }
}

// 0012C830
void mgCTextureManager::SetTableBuffer(int n_textures, int n_blocks, mgCMemory* stack)
{
  log_trace("mgCTextureBlock::SetTableBuffer({}, {}, {})", n_textures, n_blocks, fmt::ptr(stack));

  m_n_blocks = n_blocks;
  mgCTextureBlock* block_buf = static_cast<mgCTextureBlock*>(
    stack->Alloc(
      BYTES_TO_BLOCKS(
        sizeof(mgCTextureBlock) * n_blocks
      )
    )
  );

  m_blocks = block_buf;
  if (block_buf != nullptr)
  {
    for (int i = 0; i < n_blocks; ++i)
    {
      new (&block_buf[i]) mgCTextureBlock;
    }
  }
  else
  {
    m_n_blocks = 0;
  }

  m_n_textures = n_textures;
  mgCTexture* texture_buf = static_cast<mgCTexture*>(
    stack->Alloc(
      BYTES_TO_BLOCKS(
        sizeof(mgCTexture) * n_textures
      )
    )
  );

  m_textures = texture_buf;
  if (texture_buf != nullptr)
  {
    for (int i = 0; i < n_textures; ++i)
    {
      new (&texture_buf[i]) mgCTexture;
    }
  }

  mgCTexture** p_texture_buf = static_cast<mgCTexture**>(
    stack->Alloc(
      BYTES_TO_BLOCKS_STRICT(
        sizeof(mgCTexture*) * n_textures
      )
    )
  );

  // BUG: no null check
  for (int i = 0; i < m_n_textures; ++i)
  {
    m_p_textures[i] = &m_textures[i];
  }

  m_unk_field_1C4 = 0;
  m_unk_field_1D0 = m_n_textures;

  _UNKNOWNSTRUCT(0x8)* unk_buf = static_cast<_UNKNOWNSTRUCT(0x8)*>(
    stack->Alloc(
      BYTES_TO_BLOCKS(
        sizeof(_UNKNOWNSTRUCT(0x8)) * m_unk_field_1D0
      )
    )
  );

  m_unk_field_1C8 = unk_buf;

  _UNKNOWNSTRUCT(0x8)** p_unk_buf = static_cast<_UNKNOWNSTRUCT(0x8)**>(
    stack->Alloc(
      BYTES_TO_BLOCKS(
        sizeof(_UNKNOWNSTRUCT(0x8)*) * m_unk_field_1D0
      )
    )
  );

  m_unk_field_1CC = p_unk_buf;

  for (int i = 0; i < m_n_textures; ++i)
  {
    m_unk_field_1CC[i] = &m_unk_field_1C8[i];
  }

  m_unk_field_1D4 = 0;
}

// 0012CC70
u8 mgCTextureManager::hash(const char* str) const
{
  log_trace("mgCTextureManager::hash({})", str);

  int hash = 0;

  for (char c = *str; c != '\0'; c = *(++str))
  {
    hash &= 0xFF;
    hash <<= 8;
    hash += c;
    hash %= m_textures_bucket.size();
  }

  return hash & 0xFF;
}


// 0012D050
mgCTexture* mgCTextureManager::GetTexture(const char* name, ssize i)
{
  log_trace("mgCTextureManager::{}({}, {})", __func__, name, i);

  todo;
  return nullptr;
}

// 0012D140
void mgCTextureManager::EnterTexture(usize texb, const char* name, _UNKNOWNPOINTER p1, s32 width, s32 height, u32 i3, _UNKNOWNPOINTER p2, u64 i4, int i5)
{
  log_trace(
    "mgCTextureManager::{}({}, {}, {}, {}, {}, {}, {}, {}, {})",
    __func__,
    texb,
    name,
    fmt::ptr(p1),
    width,
    height,
    i3,
    fmt::ptr(p2),
    i4,
    i5
  );

  todo;
}

// 0012DA90
void mgCTextureManager::EnterIMGFile(u8* img_file_buff, usize texb, mgCMemory* stack, mgCEnterIMGInfo* enter_info)
{
  log_trace("mgCTextureManager::{}({}, {}, {}, {})", __func__, fmt::ptr(img_file_buff), texb, fmt::ptr(stack), fmt::ptr(enter_info));

  todo;
}

// 0012E540
void mgCTextureManager::DeleteBlock(usize texb)
{
  log_trace("mgCTextureManager::DeleteBlock({})", texb);

  todo;
}

// 0012E850
void mgCTextureManager::ReloadTexture(usize texb, sceVif1Packet* vif1_packet)
{
  log_trace("mgCTextureManager::ReloadTexture({}, {})", texb, fmt::ptr(vif1_packet));

  todo;
}

// 0013d8c0
void mgCTextureManager::LoadCFGFile(const char* file, int size, mgCMemory* tex_anime_stack, mgCTextureAnime* tex_anime)
{
  CScriptInterpreter interpreter{ };

  pTexAnime = nullptr;
  group_name = nullptr;
  ta_enable = 0;
  now_texb = 0xffffffff;
  now_group = 0xffffffff;
  texBugPatch = 0;

  pLoadTexAnime = tex_anime;
  TexManager = this;
  TexAnimeStack = tex_anime_stack;

  interpreter.SetTag(tex_tag.data());
  interpreter.SetScript(file, size);
  interpreter.Run();
}

// 0012C480
mgCTexture::mgCTexture()
{
  log_trace("mgCTexture::mgCTexture()");

  Initialize();
}

// 0012C4B0
void mgCTexture::Initialize()
{
  log_trace("mgCTexture::Initialize()");

  m_name[0] = '\0';
  m_hash_uuid = -1;

  for (float& f : m_unk_field_50)
  {
    f = 0.0f;
  }

  m_unk_field_60 = 0;
  m_unk_field_48 = 0;
  m_unk_field_40 = 0;
  m_unk_field_38 = 0;

  m_unk_field_48 = m_unk_field_48 & (~0b0000'0011) | 1;
  m_unk_field_48 = m_unk_field_48 & (~0b0000'1100) | 4;

  m_unk_field_6 = 0;
  m_unk_field_4 = 0;
  m_unk_field_2 = 0;
  m_unk_field_64 = 0;
  m_unk_field_28 = 0;
  m_unk_field_2C = 0;
  m_unk_field_30 = 0;
  m_next = nullptr;
}