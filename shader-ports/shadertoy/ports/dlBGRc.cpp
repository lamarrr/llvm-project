// Port of https://www.shadertoy.com/view/dlBGRc; author: toomuchvoltage.
// Dataset license metadata: mit; original retained in upstream/dlBGRc.json.
#include "../support.h"
using namespace glsl;
struct Texture2D {
  texture_2d<f32x4> image;
  sampler sampling;
};
f32x4 texture(Texture2D t, f32x2 uv) {
  return t.image.sample(t.sampling, uv.data[0], uv.data[1]);
}
f32x4 textureLod(Texture2D t, f32x2 uv, float lod) {
  return t.image.sample_lod(t.sampling, uv.data[0], uv.data[1], lod);
}
i32x2 textureSize(Texture2D t, int lod) {
  auto size = t.image.dimensions(static_cast<unsigned>(lod));
  return i32x2(size[0], size[1]);
}
f32x4 texelFetch(Texture2D t, i32x2 uv) {
  return t.image.load(uv.data[0], uv.data[1]);
}
struct Texture3D {
  texture_3d<f32x4> image;
  sampler sampling;
};
f32x4 texture(Texture3D t, f32x3 uv) {
  return t.image.sample(t.sampling, uv.data[0], uv.data[1], uv.data[2]);
}
f32x4 textureLod(Texture3D t, f32x3 uv, float lod) {
  return t.image.sample_lod(t.sampling, uv.data[0], uv.data[1], uv.data[2],
                            lod);
}
i32x3 textureSize(Texture3D t, int lod) {
  auto size = t.image.dimensions(static_cast<unsigned>(lod));
  return i32x3(size[0], size[1], size[2]);
}
f32x4 texelFetch(Texture3D t, i32x3 uv) {
  return t.image.load(uv.data[0], uv.data[1], uv.data[2]);
}
struct TextureCube {
  texture_cube<f32x4> image;
  sampler sampling;
};
f32x4 texture(TextureCube t, f32x3 uv) {
  return t.image.sample(t.sampling, uv.data[0], uv.data[1], uv.data[2]);
}
f32x4 textureLod(TextureCube t, f32x3 uv, float lod) {
  return t.image.sample_lod(t.sampling, uv.data[0], uv.data[1], uv.data[2],
                            lod);
}
i32x2 textureSize(TextureCube t, int lod) {
  auto size = t.image.dimensions(static_cast<unsigned>(lod));
  return i32x2(size[0], size[1]);
}
using sampler2D = Texture2D;
using sampler3D = Texture3D;
using samplerCube = TextureCube;
struct Context {};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
bool rayBoxIntersectTime(Context &__ctx, f32x3 l1, f32x3 invm, f32x3 bmin,
                         f32x3 bmax, float &tMin, float &tMax) {
  f32x3 bmin_l1 = (bmin - l1) * invm;
  f32x3 bmax_l1 = (bmax - l1) * invm;
  f32x3 minVec = min(bmin_l1, bmax_l1);
  f32x3 maxVec = max(bmin_l1, bmax_l1);
  float tmin = max(max(minVec.data[0], minVec.data[1]), minVec.data[2]);
  float tmax = min(min(maxVec.data[0], maxVec.data[1]), maxVec.data[2]);
  bool retVal = ((tmax >= tmin) && (tmin < 1.0f) && (tmax > 0.0f));
  tMin = tmin;
  tMax = tmax;
  return retVal;
}
unsigned countSetBits(Context &__ctx, unsigned n) {
  unsigned count = 0u;
  while (n != 0u) {
    count += (n & 1u);
    n >>= 1u;
  }
  return count;
}
unsigned countSetBitsBefore(Context &__ctx, unsigned n, unsigned comp) {
  unsigned beforeMask = comp ^ (comp - 1u);
  n &= (~beforeMask);
  unsigned count = 0u;
  while (n != 0u) {
    count += (n & 1u);
    n >>= 1u;
  }
  return count;
}
const f32x3 grid0Min = f32x3(-4.00f, -4.00f, -4.00f);
const f32x3 grid0Max = f32x3(4.00f, 4.00f, 4.00f);
const f32x3 grid0Range = grid0Max - grid0Min;
unsigned svoObject0[304] = unsigned[](
    1052696u, 404241420u, 202117616u, 270000383u, 2414872600u, 2021648262u,
    116657212u, 1070856002u, 509478378u, 336916560u, 4280307135u, 2423814248u,
    2106053692u, 1010579521u, 3256871445u, 4269151568u, 274069423u, 3237995263u,
    2265437170u, 4089692931u, 4194171361u, 3783360943u, 732929282u, 1420431477u,
    4030070746u, 2140676349u, 126945219u, 3285173239u, 3888111390u, 471918478u,
    242322062u, 1086341601u, 3768705505u, 4042322160u, 3230294630u, 2141139762u,
    818925336u, 295506050u, 2206614275u, 3282108800u, 366276676u, 1609359359u,
    4286079078u, 1726373503u, 1009259519u, 4196479902u, 504756729u, 2677869226u,
    135491603u, 4048780943u, 4027064294u, 1643764743u, 4094430195u, 4107046674u,
    1719010746u, 1143644200u, 2867592847u, 3892979843u, 2880002027u,
    3129551370u, 44705952u, 2884287275u, 2914126370u, 2884336802u, 4010322464u,
    2157966426u, 538619314u, 4891290u, 2550920895u, 4207911431u, 2863722564u,
    73819750u, 1644566394u, 2951383799u, 265818130u, 3188338223u, 4037010920u,
    2139155072u, 4286622599u, 2944931960u, 2021654463u, 1010326588u,
    1010581502u, 504391422u, 505290271u, 3893575404u, 3473886222u, 1905892825u,
    2556899532u, 3435978648u, 4019256576u, 4279238054u, 1719171168u,
    2066742048u, 808466457u, 2415929824u, 3154083993u, 3220044996u, 1538252029u,
    3202770531u, 2254727123u, 51634113u, 872126872u, 1659738554u, 4020808328u,
    2854232639u, 2707392488u, 89844230u, 117666755u, 3947500937u, 1467984426u,
    2935312125u, 1476952746u, 2852825771u, 2720180906u, 716528565u, 1474472490u,
    4004183432u, 2852661384u, 2291907492u, 3409624131u, 3105362329u,
    2622233807u, 3153899344u, 3208703760u, 269819205u, 286259576u, 2105080326u,
    26281968u, 1023407043u, 33800689u, 20977141u, 4126220005u, 4195350787u,
    4290527981u, 318506755u, 3280142592u, 348144860u, 4232070768u, 1071504313u,
    3061709759u, 3737858867u, 590560867u, 507091359u, 859783118u, 3485177740u,
    3865494625u, 4294773487u, 147630711u, 2286239813u, 4290724067u, 3825179920u,
    437234178u, 2876635105u, 2702159618u, 4245312835u, 3286937278u, 522066804u,
    44612266u, 2175732527u, 4039069943u, 1091779497u, 3042409316u, 1466081826u,
    4008414890u, 3884746792u, 2290654139u, 3025128244u, 1306106329u,
    2644090060u, 3752972223u, 3706654992u, 3810334720u, 572531263u, 4045291239u,
    1896038398u, 3981809473u, 67120096u, 2692759520u, 3154115583u, 4106801122u,
    262671365u, 4457597u, 3401293258u, 3114130627u, 3007034174u, 1637456379u,
    3878735054u, 4109342822u, 3429292539u, 3718282626u, 4208796245u,
    1575720317u, 1628504533u, 1353428923u, 1432185488u, 715910081u, 1440044373u,
    1431385973u, 1067869248u, 2538038357u, 1646438955u, 3996380810u, 871279144u,
    2308684723u, 617329196u, 3275329945u, 2165621956u, 3435654689u, 3486257390u,
    4010806880u, 644245095u, 842015539u, 807536243u, 254279742u, 251649983u,
    403177532u, 3927826174u, 3929398511u, 3486317792u, 2389075462u, 1615278867u,
    4046664602u, 2140772798u, 2881137375u, 581089876u, 68515157u, 1365112289u,
    3741629023u, 3751829410u, 1410684277u, 1094800887u, 2707382111u, 95049128u,
    1343241143u, 2757229205u, 1538269218u, 581857262u, 937986175u, 2156406922u,
    2820403756u, 3283857452u, 1339627793u, 3650899020u, 3439320067u,
    4243467093u, 3624822374u, 1644562227u, 857932288u, 8718u, 1062274844u,
    2028019454u, 1719034782u, 2134324087u, 1858868213u, 3831503860u,
    1098268330u, 350885346u, 169406671u, 448688354u, 3200134079u, 1075836320u,
    2695904938u, 99227114u, 168885872u, 2733671102u, 168435700u, 536191226u,
    4197095352u, 2283466923u, 3218128203u, 3625547900u, 471612670u, 4264431580u,
    3218206977u, 2699591709u, 1296301069u, 266344478u, 3890718695u, 4027342835u,
    3019105232u, 4244701440u, 17u, 391188377u, 2174644323u, 2409614536u,
    2167619532u, 3393283093u, 1542931716u, 159243640u, 2112454140u, 273013823u,
    1820195669u, 3578872064u, 353308431u, 3886415751u, 2181457907u, 4226008003u,
    839074113u, 1075055887u, 2273805830u, 126075843u, 50397184u, 0u);
unsigned readBitsSVO0(Context &__ctx, unsigned bitLoc, unsigned numBits) {
  unsigned wordLoc = bitLoc / 32u;
  unsigned leftToRead = (32u - (bitLoc % 32u));
  if (numBits <= leftToRead) {
    unsigned shiftToMask = leftToRead - numBits;
    unsigned masker = 0xFFFFFFFFu;
    masker >>= unsigned(32u - numBits);
    masker <<= shiftToMask;
    unsigned value = (svoObject0[wordLoc] & masker);
    value >>= shiftToMask;
    return value;
  } else {
    unsigned bottomBits = numBits - leftToRead;
    unsigned masker = 0xFFFFFFFFu;
    masker >>= unsigned(32u - leftToRead);
    unsigned topNum = (svoObject0[wordLoc] & masker);
    unsigned bottomMasker = 0xFFFFFFFFu;
    unsigned bottomShifter = unsigned(32u - bottomBits);
    bottomMasker <<= bottomShifter;
    unsigned value = (svoObject0[wordLoc + 1u] & bottomMasker);
    unsigned bottomNum = (value >> bottomShifter);
    return ((topNum << bottomBits) | bottomNum);
  }
}
bool readLeafSVO0(Context &__ctx, f32x3 samplePos, f32x3 sampleDir,
                  f32x3 &skipPos) {
  skipPos = f32x3(10000.0f);
  if (any(less_than(samplePos, grid0Min)) ||
      any(greater_than(samplePos, grid0Max)))
    return false;
  u32x3 topBrickPos = u32x3(samplePos - grid0Min);
  unsigned topBrickId = topBrickPos.data[2] +
                        topBrickPos.data[1] * unsigned(grid0Range.data[0]) +
                        topBrickPos.data[0] * unsigned(grid0Range.data[1]) *
                            unsigned(grid0Range.data[2]);
  unsigned streamReadPos = 0u;
  for (int i = 0; i < int(topBrickId); i++) {
    unsigned isOcc = readBitsSVO0(__ctx, streamReadPos, 1u);
    streamReadPos += 1u;
    if (isOcc == 1u) {
      unsigned countMidBricks =
          countSetBits(__ctx, readBitsSVO0(__ctx, streamReadPos, 8u));
      streamReadPos += (8u + countMidBricks * 8u);
    }
  }
  unsigned topBrick = readBitsSVO0(__ctx, streamReadPos, 1u);
  if (topBrick == 0u) {
    f32x3 topBrickMin = grid0Min + f32x3(topBrickPos);
    f32x3 topBrickMax = topBrickMin + f32x3(1.0f);
    f32x3 p1 = samplePos;
    f32x3 p2 = p1 + sampleDir * 2.0f;
    f32x3 m = p2 - p1;
    float tMin, tMax;
    rayBoxIntersectTime(__ctx, p1, f32x3(1.0f) / m, topBrickMin, topBrickMax,
                        tMin, tMax);
    skipPos = p1 + m * (tMax + 0.01f);
    return false;
  }
  streamReadPos += 1u;
  unsigned midBricks = readBitsSVO0(__ctx, streamReadPos, 8u);
  streamReadPos += 8u;
  f32x3 topBrickMinCorner = grid0Min + f32x3(topBrickPos);
  f32x3 sampleRelativeToTopBrick = fract(samplePos);
  unsigned checkMidBrickBit = 0x80u;
  f32x3 sampleRelativeToMidBrick = sampleRelativeToTopBrick;
  f32x3 midBrickPos = f32x3(0.0f);
  if (sampleRelativeToTopBrick.data[0] > 0.5f) {
    sampleRelativeToMidBrick.data[0] -= 0.5f;
    midBrickPos.data[0] = 0.5f;
    checkMidBrickBit >>= 4u;
  }
  if (sampleRelativeToTopBrick.data[1] > 0.5f) {
    sampleRelativeToMidBrick.data[1] -= 0.5f;
    midBrickPos.data[1] = 0.5f;
    checkMidBrickBit >>= 2u;
  }
  if (sampleRelativeToTopBrick.data[2] > 0.5f) {
    sampleRelativeToMidBrick.data[2] -= 0.5f;
    midBrickPos.data[2] = 0.5f;
    checkMidBrickBit >>= 1u;
  }
  if ((midBricks & checkMidBrickBit) == 0u) {
    f32x3 midBrickMin = grid0Min + f32x3(topBrickPos) + midBrickPos;
    f32x3 midBrickMax = midBrickMin + f32x3(0.5f);
    f32x3 p1 = samplePos;
    f32x3 p2 = p1 + sampleDir * 2.0f;
    f32x3 m = p2 - p1;
    float tMin, tMax;
    rayBoxIntersectTime(__ctx, p1, f32x3(1.0f) / m, midBrickMin, midBrickMax,
                        tMin, tMax);
    skipPos = p1 + m * (tMax + 0.01f);
    return false;
  }
  unsigned skipMidBricks =
      countSetBitsBefore(__ctx, midBricks, checkMidBrickBit);
  streamReadPos += (8u * skipMidBricks);
  unsigned finalMidBrick = readBitsSVO0(__ctx, streamReadPos, 8u);
  unsigned checkVoxelBrickBit = 0x80u;
  if (sampleRelativeToMidBrick.data[0] > 0.25f) {
    checkVoxelBrickBit >>= 4u;
  }
  if (sampleRelativeToMidBrick.data[1] > 0.25f) {
    checkVoxelBrickBit >>= 2u;
  }
  if (sampleRelativeToMidBrick.data[2] > 0.25f) {
    checkVoxelBrickBit >>= 1u;
  }
  if ((checkVoxelBrickBit & finalMidBrick) != 0u)
    return true;
  skipPos = samplePos + sampleDir * 0.25f;
  return false;
}
bool traceRaySVO0(Context &__ctx, f32x3 p1, f32x3 p2, f32x3 &hitPos) {
  f32x3 m = p2 - p1;
  float hitMin, hitMax;
  if (!rayBoxIntersectTime(__ctx, p1, f32x3(1.0f) / m, grid0Min, grid0Max,
                           hitMin, hitMax)) {
    hitPos = f32x3(-1.0f);
    return false;
  }
  hitMin += 0.00001f;
  hitMax -= 0.00001f;
  f32x3 curPos = p1 + hitMin * m;
  f32x3 curDir = normalize(m);
  f32x3 skipPos = f32x3(0.0f);
  for (int i = 0; i != 100; i++) {
    if (readLeafSVO0(__ctx, curPos, curDir, skipPos)) {
      hitPos = curPos;
      return true;
    }
    if (skipPos == f32x3(10000.0f))
      break;
    curPos = skipPos;
  }
  return false;
}
const f32x3 grid1Min = f32x3(-4.00f, -4.00f, -4.00f);
const f32x3 grid1Max = f32x3(4.00f, 4.00f, 4.00f);
const f32x3 grid1Range = grid1Max - grid1Min;
unsigned svoObject1[377] = unsigned[](
    1136706755u, 1138876512u, 1105285168u, 276306008u, 404758704u, 4042322160u,
    4168644728u, 2021402664u, 673218704u, 4042322112u, 4168644728u, 2019827752u,
    272646161u, 806359865u, 412719516u, 202116612u, 1712648256u, 1303392383u,
    4269799458u, 1805521414u, 1744632895u, 4240129919u, 4079161373u,
    3693117589u, 4029734911u, 3561872888u, 276304280u, 196602940u, 470547640u,
    174080614u, 1680015528u, 136347596u, 3438235328u, 1154973247u, 3328599662u,
    1181736351u, 3253993228u, 214749153u, 4194174854u, 1718023183u, 4096521439u,
    1109392889u, 3789623293u, 49332055u, 3225671166u, 392740609u, 4283170328u,
    403179479u, 1119699641u, 1096162759u, 2652179660u, 3427860791u, 2135555648u,
    2605181619u, 184682881u, 2206719942u, 731653021u, 1579290640u, 736747658u,
    3020471903u, 1061093620u, 4060286911u, 2268622944u, 2173156904u, 733618211u,
    3760241122u, 1725166387u, 1018966208u, 2314312094u, 2017526984u,
    3435677760u, 1081073824u, 577503119u, 4294966779u, 858836940u, 3472683001u,
    2164155974u, 1743152751u, 3103779075u, 4261552193u, 1094795232u,
    2581627323u, 1410037375u, 3951026429u, 4241277711u, 4193907199u,
    3865471903u, 4294686443u, 3005480578u, 2130320u, 3435974214u, 67387554u,
    2158928507u, 2680104691u, 858716559u, 3650722204u, 3435977932u, 3462821478u,
    1718056821u, 3425924239u, 67080094u, 404227621u, 2314730205u, 2685903088u,
    3766452034u, 3509539647u, 3513148768u, 1447298060u, 1560271906u,
    1742487537u, 3519577984u, 2152730687u, 3130499612u, 3787055643u,
    4037050459u, 859819791u, 3758626936u, 4187748336u, 2113659395u, 587399440u,
    1039065503u, 3256761585u, 3506290432u, 4244559978u, 4227663761u, 100827221u,
    1094800477u, 4146028382u, 299065103u, 282074971u, 2371123922u, 3640689280u,
    2197782615u, 3244203490u, 3124501528u, 337579020u, 1124274174u, 1728024442u,
    1833086214u, 41976458u, 2189411710u, 92203520u, 4151256422u, 1615323316u,
    1006744127u, 4263966879u, 1387070204u, 100827251u, 50477088u, 971020715u,
    2800401884u, 3435974316u, 1547961968u, 2821237500u, 2096942019u,
    3426614521u, 3238137374u, 956103884u, 3474927856u, 4269696623u, 2172813279u,
    1065038799u, 1044135933u, 3109322577u, 3208699007u, 2011698540u,
    2419944417u, 2447476840u, 1618509820u, 1441921536u, 2142890499u, 298909815u,
    3241591748u, 1788176446u, 369499134u, 386074382u, 1069123516u, 2017720455u,
    1445077504u, 1405157758u, 2165768033u, 3957362660u, 504635262u, 1895493761u,
    51321968u, 1202214008u, 1163133761u, 1072577404u, 3925573121u, 4278189835u,
    263130942u, 1894828096u, 1621629096u, 2081168444u, 1010800097u, 4125220993u,
    2683370992u, 4031841029u, 252511639u, 3500703741u, 4164828208u, 1041743642u,
    505412592u, 1548473679u, 2215415047u, 1161843008u, 1146355039u, 4282488409u,
    335875073u, 4210785u, 3760218595u, 4092723968u, 808644856u, 2014681214u,
    4286321704u, 550424379u, 865995280u, 4282751248u, 1197212163u, 3288329758u,
    473555455u, 1105854463u, 252509438u, 1357967613u, 4287075584u, 2021154916u,
    540564796u, 1415823677u, 3789676993u, 3825082013u, 4042322130u, 4035906319u,
    2262631165u, 7984451u, 2507936889u, 3791551073u, 3786473263u, 16629586u,
    4244826373u, 8361320u, 25089346u, 358876695u, 1090719504u, 3713481813u,
    1431327571u, 1366070079u, 2055086481u, 2216389758u, 4269334657u, 125618376u,
    1145053313u, 4466688u, 4286873899u, 3757677038u, 2145304655u, 3750245307u,
    1037032848u, 242864412u, 3704474829u, 3494903781u, 3926263542u, 2052073468u,
    4294955839u, 989082975u, 4290381214u, 98910710u, 1084370848u, 2147280917u,
    1748976277u, 1120999168u, 1509900248u, 2040526343u, 4257413070u,
    1020018644u, 1071521300u, 471824057u, 4211255935u, 3514597683u, 3577976138u,
    906370654u, 525406047u, 418402573u, 1347437909u, 1463554097u, 66904046u,
    534897639u, 487066199u, 3276018044u, 2172567105u, 4058507252u, 527495410u,
    4035249423u, 142430341u, 1923890175u, 3149533200u, 2072279445u, 1434064240u,
    295455951u, 3171505493u, 1431336789u, 1219695946u, 4045829734u, 2007168881u,
    377275262u, 3431744597u, 1077512337u, 135331840u, 1544354875u, 2146434945u,
    3766361336u, 513801420u, 3224382716u, 3419651942u, 1717634662u, 2134061875u,
    926906226u, 3927275017u, 1416892476u, 1010698223u, 4263386959u, 3420393220u,
    3472882072u, 1726550031u, 3620523468u, 807650247u, 4288549361u, 1617122766u,
    274471940u, 1040088638u, 1910628878u, 524058528u, 3451851385u, 2097166968u,
    2021489091u, 3275504699u, 4058112481u, 1080404u, 601179028u, 836836894u,
    1041561076u, 3495815925u, 1610446167u, 1350062144u, 2098528980u, 735323616u,
    1577156119u, 4159721489u, 4294938600u, 218453312u, 1346191239u, 3799654280u,
    505316351u, 2349334528u, 1904399u, 219121025u, 2273779648u, 3234008846u,
    1642348559u, 2393406392u, 1938279484u, 1009795010u, 63029056u, 4229367633u,
    519961976u, 1488684934u, 2265442325u, 4261290946u, 50401322u, 34488078u,
    1895430095u, 16741952u, 716569506u, 169866698u, 2804596498u, 737914908u,
    3453680975u, 4026531840u);
unsigned readBitsSVO1(Context &__ctx, unsigned bitLoc, unsigned numBits) {
  unsigned wordLoc = bitLoc / 32u;
  unsigned leftToRead = (32u - (bitLoc % 32u));
  if (numBits <= leftToRead) {
    unsigned shiftToMask = leftToRead - numBits;
    unsigned masker = 0xFFFFFFFFu;
    masker >>= unsigned(32u - numBits);
    masker <<= shiftToMask;
    unsigned value = (svoObject1[wordLoc] & masker);
    value >>= shiftToMask;
    return value;
  } else {
    unsigned bottomBits = numBits - leftToRead;
    unsigned masker = 0xFFFFFFFFu;
    masker >>= unsigned(32u - leftToRead);
    unsigned topNum = (svoObject1[wordLoc] & masker);
    unsigned bottomMasker = 0xFFFFFFFFu;
    unsigned bottomShifter = unsigned(32u - bottomBits);
    bottomMasker <<= bottomShifter;
    unsigned value = (svoObject1[wordLoc + 1u] & bottomMasker);
    unsigned bottomNum = (value >> bottomShifter);
    return ((topNum << bottomBits) | bottomNum);
  }
}
bool readLeafSVO1(Context &__ctx, f32x3 samplePos, f32x3 sampleDir,
                  f32x3 &skipPos) {
  skipPos = f32x3(10000.0f);
  if (any(less_than(samplePos, grid1Min)) ||
      any(greater_than(samplePos, grid1Max)))
    return false;
  u32x3 topBrickPos = u32x3(samplePos - grid1Min);
  unsigned topBrickId = topBrickPos.data[2] +
                        topBrickPos.data[1] * unsigned(grid0Range.data[0]) +
                        topBrickPos.data[0] * unsigned(grid0Range.data[1]) *
                            unsigned(grid0Range.data[2]);
  unsigned streamReadPos = 0u;
  for (int i = 0; i < int(topBrickId); i++) {
    unsigned isOcc = readBitsSVO1(__ctx, streamReadPos, 1u);
    streamReadPos += 1u;
    if (isOcc == 1u) {
      unsigned countMidBricks =
          countSetBits(__ctx, readBitsSVO1(__ctx, streamReadPos, 8u));
      streamReadPos += (8u + countMidBricks * 8u);
    }
  }
  unsigned topBrick = readBitsSVO1(__ctx, streamReadPos, 1u);
  if (topBrick == 0u) {
    f32x3 topBrickMin = grid1Min + f32x3(topBrickPos);
    f32x3 topBrickMax = topBrickMin + f32x3(1.0f);
    f32x3 p1 = samplePos;
    f32x3 p2 = p1 + sampleDir * 2.0f;
    f32x3 m = p2 - p1;
    float tMin, tMax;
    rayBoxIntersectTime(__ctx, p1, f32x3(1.0f) / m, topBrickMin, topBrickMax,
                        tMin, tMax);
    skipPos = p1 + m * (tMax + 0.01f);
    return false;
  }
  streamReadPos += 1u;
  unsigned midBricks = readBitsSVO1(__ctx, streamReadPos, 8u);
  streamReadPos += 8u;
  f32x3 topBrickMinCorner = grid1Min + f32x3(topBrickPos);
  f32x3 sampleRelativeToTopBrick = fract(samplePos);
  unsigned checkMidBrickBit = 0x80u;
  f32x3 sampleRelativeToMidBrick = sampleRelativeToTopBrick;
  f32x3 midBrickPos = f32x3(0.0f);
  if (sampleRelativeToTopBrick.data[0] > 0.5f) {
    sampleRelativeToMidBrick.data[0] -= 0.5f;
    midBrickPos.data[0] = 0.5f;
    checkMidBrickBit >>= 4u;
  }
  if (sampleRelativeToTopBrick.data[1] > 0.5f) {
    sampleRelativeToMidBrick.data[1] -= 0.5f;
    midBrickPos.data[1] = 0.5f;
    checkMidBrickBit >>= 2u;
  }
  if (sampleRelativeToTopBrick.data[2] > 0.5f) {
    sampleRelativeToMidBrick.data[2] -= 0.5f;
    midBrickPos.data[2] = 0.5f;
    checkMidBrickBit >>= 1u;
  }
  if ((midBricks & checkMidBrickBit) == 0u) {
    f32x3 midBrickMin = grid1Min + f32x3(topBrickPos) + midBrickPos;
    f32x3 midBrickMax = midBrickMin + f32x3(0.5f);
    f32x3 p1 = samplePos;
    f32x3 p2 = p1 + sampleDir * 2.0f;
    f32x3 m = p2 - p1;
    float tMin, tMax;
    rayBoxIntersectTime(__ctx, p1, f32x3(1.0f) / m, midBrickMin, midBrickMax,
                        tMin, tMax);
    skipPos = p1 + m * (tMax + 0.01f);
    return false;
  }
  unsigned skipMidBricks =
      countSetBitsBefore(__ctx, midBricks, checkMidBrickBit);
  streamReadPos += (8u * skipMidBricks);
  unsigned finalMidBrick = readBitsSVO1(__ctx, streamReadPos, 8u);
  unsigned checkVoxelBrickBit = 0x80u;
  if (sampleRelativeToMidBrick.data[0] > 0.25f) {
    checkVoxelBrickBit >>= 4u;
  }
  if (sampleRelativeToMidBrick.data[1] > 0.25f) {
    checkVoxelBrickBit >>= 2u;
  }
  if (sampleRelativeToMidBrick.data[2] > 0.25f) {
    checkVoxelBrickBit >>= 1u;
  }
  if ((checkVoxelBrickBit & finalMidBrick) != 0u)
    return true;
  skipPos = samplePos + sampleDir * 0.25f;
  return false;
}
bool traceRaySVO1(Context &__ctx, f32x3 p1, f32x3 p2, f32x3 &hitPos) {
  f32x3 m = p2 - p1;
  float hitMin, hitMax;
  if (!rayBoxIntersectTime(__ctx, p1, f32x3(1.0f) / m, grid1Min, grid1Max,
                           hitMin, hitMax)) {
    hitPos = f32x3(-1.0f);
    return false;
  }
  hitMin += 0.00001f;
  hitMax -= 0.00001f;
  f32x3 curPos = p1 + hitMin * m;
  f32x3 curDir = normalize(m);
  f32x3 skipPos = f32x3(0.0f);
  for (int i = 0; i != 100; i++) {
    if (readLeafSVO1(__ctx, curPos, curDir, skipPos)) {
      hitPos = curPos;
      return true;
    }
    if (skipPos == f32x3(10000.0f))
      break;
    curPos = skipPos;
  }
  return false;
}
const f32x3 grid2Min = f32x3(-4.00f, -4.00f, -4.00f);
const f32x3 grid2Max = f32x3(4.00f, 4.00f, 4.00f);
const f32x3 grid2Range = grid2Max - grid2Min;
unsigned svoObject2[254] = unsigned[](
    273480800u, 1207793664u, 152u, 161074380u, 3317964262u, 3890733177u,
    3650236971u, 3505455015u, 4287033240u, 947648639u, 4265557243u, 4234936271u,
    4278066745u, 4186050553u, 2565517168u, 3363865130u, 4077060667u,
    2684361796u, 1308581887u, 4294452863u, 4268142550u, 3200003954u,
    2882840592u, 287305768u, 2021161980u, 123124796u, 1027779014u, 2819969104u,
    1163223032u, 2020636796u, 1132824454u, 740540156u, 135282766u, 2015381471u,
    2552259643u, 3156750336u, 1141124836u, 1052361218u, 507503411u, 1015890739u,
    955119645u, 2018613522u, 91771924u, 289382368u, 2107214366u, 516374392u,
    200490084u, 2008834048u, 260736824u, 2407237627u, 1073493675u, 3263348753u,
    17825592u, 1163401863u, 2273806312u, 380198531u, 3279951125u, 357900157u,
    2020098439u, 2675468015u, 3981618377u, 921179205u, 1432213782u, 1876514777u,
    131875519u, 3727422659u, 3155177783u, 4227358744u, 444585959u, 3325830102u,
    1073004036u, 69247008u, 2665550847u, 1609437102u, 3791257220u, 34075900u,
    804437763u, 4047656453u, 1610220576u, 2686492093u, 4222671788u, 4290746775u,
    2256437145u, 2279589566u, 1008812483u, 3743158750u, 287064133u, 1642111325u,
    3780133064u, 1651407923u, 1009778688u, 159614908u, 3352068102u, 116391695u,
    24379374u, 290804063u, 4016535653u, 1207332725u, 871192443u, 4043463701u,
    4279850015u, 4027971564u, 2166269816u, 218432517u, 1094796117u, 1434417146u,
    2156511437u, 4289391272u, 2853695482u, 2829757098u, 2214308436u,
    2706680645u, 1417815671u, 1090732851u, 4273438687u, 4026269272u, 402938634u,
    2197947456u, 1139712853u, 3994113187u, 1414792277u, 1072693183u,
    4120880760u, 270515711u, 809550644u, 1288675225u, 2556454502u, 1740230556u,
    809881395u, 286907400u, 135019580u, 1010581008u, 336079096u, 2084355132u,
    1008734224u, 1509427224u, 947648484u, 3959963695u, 4104806625u, 3777032213u,
    1361015125u, 1599567456u, 1196513143u, 863835395u, 4253009377u, 22298655u,
    1897592688u, 2266463464u, 1433953301u, 2105496951u, 4282512520u, 416087547u,
    3289132672u, 3212732954u, 2863311594u, 3015621205u, 1438531723u,
    1419797111u, 2131490640u, 875576801u, 2173009470u, 2123886778u, 4092766224u,
    335808253u, 1412125739u, 1425948996u, 1094319031u, 2130704889u, 3754160180u,
    214895613u, 4162741857u, 4194174471u, 3092045616u, 4294050577u, 3641709632u,
    1305475839u, 4163108863u, 2047340543u, 3006767443u, 1072349039u,
    1163220032u, 269557752u, 2406677890u, 2407495757u, 4258580412u, 964448251u,
    286326805u, 1364546901u, 1601689092u, 68167766u, 4023718571u, 4252925972u,
    1163218425u, 401169912u, 505931259u, 3120472446u, 2746225409u, 119505543u,
    3885998019u, 3800288970u, 1077944191u, 1560105127u, 1884991080u,
    1617422296u, 805059964u, 272547971u, 3934470501u, 4095692754u, 3155156957u,
    4161927673u, 3690857989u, 4231855615u, 3800236547u, 1106575428u, 339608084u,
    519377932u, 235275815u, 52625428u, 89024487u, 4194567683u, 3893428491u,
    2172880775u, 3284304834u, 1312015u, 2273806215u, 2256568960u, 235308934u,
    41943074u, 3768721599u, 4293990261u, 4194513724u, 3235831773u, 2465831677u,
    75628585u, 2692779001u, 4243913931u, 259846286u, 1206339168u, 0u, 11874u,
    251249166u, 993201359u, 3208315008u, 2287845440u, 1204617211u, 4160708438u,
    2129781759u, 3623319416u, 2164240072u, 10486399u, 257945907u, 809450910u,
    1976639376u, 0u);
unsigned readBitsSVO2(Context &__ctx, unsigned bitLoc, unsigned numBits) {
  unsigned wordLoc = bitLoc / 32u;
  unsigned leftToRead = (32u - (bitLoc % 32u));
  if (numBits <= leftToRead) {
    unsigned shiftToMask = leftToRead - numBits;
    unsigned masker = 0xFFFFFFFFu;
    masker >>= unsigned(32u - numBits);
    masker <<= shiftToMask;
    unsigned value = (svoObject2[wordLoc] & masker);
    value >>= shiftToMask;
    return value;
  } else {
    unsigned bottomBits = numBits - leftToRead;
    unsigned masker = 0xFFFFFFFFu;
    masker >>= unsigned(32u - leftToRead);
    unsigned topNum = (svoObject2[wordLoc] & masker);
    unsigned bottomMasker = 0xFFFFFFFFu;
    unsigned bottomShifter = unsigned(32u - bottomBits);
    bottomMasker <<= bottomShifter;
    unsigned value = (svoObject2[wordLoc + 1u] & bottomMasker);
    unsigned bottomNum = (value >> bottomShifter);
    return ((topNum << bottomBits) | bottomNum);
  }
}
bool readLeafSVO2(Context &__ctx, f32x3 samplePos, f32x3 sampleDir,
                  f32x3 &skipPos) {
  skipPos = f32x3(10000.0f);
  if (any(less_than(samplePos, grid2Min)) ||
      any(greater_than(samplePos, grid2Max)))
    return false;
  u32x3 topBrickPos = u32x3(samplePos - grid2Min);
  unsigned topBrickId = topBrickPos.data[2] +
                        topBrickPos.data[1] * unsigned(grid0Range.data[0]) +
                        topBrickPos.data[0] * unsigned(grid0Range.data[1]) *
                            unsigned(grid0Range.data[2]);
  unsigned streamReadPos = 0u;
  for (int i = 0; i < int(topBrickId); i++) {
    unsigned isOcc = readBitsSVO2(__ctx, streamReadPos, 1u);
    streamReadPos += 1u;
    if (isOcc == 1u) {
      unsigned countMidBricks =
          countSetBits(__ctx, readBitsSVO2(__ctx, streamReadPos, 8u));
      streamReadPos += (8u + countMidBricks * 8u);
    }
  }
  unsigned topBrick = readBitsSVO2(__ctx, streamReadPos, 1u);
  if (topBrick == 0u) {
    f32x3 topBrickMin = grid2Min + f32x3(topBrickPos);
    f32x3 topBrickMax = topBrickMin + f32x3(1.0f);
    f32x3 p1 = samplePos;
    f32x3 p2 = p1 + sampleDir * 2.0f;
    f32x3 m = p2 - p1;
    float tMin, tMax;
    rayBoxIntersectTime(__ctx, p1, f32x3(1.0f) / m, topBrickMin, topBrickMax,
                        tMin, tMax);
    skipPos = p1 + m * (tMax + 0.01f);
    return false;
  }
  streamReadPos += 1u;
  unsigned midBricks = readBitsSVO2(__ctx, streamReadPos, 8u);
  streamReadPos += 8u;
  f32x3 topBrickMinCorner = grid2Min + f32x3(topBrickPos);
  f32x3 sampleRelativeToTopBrick = fract(samplePos);
  unsigned checkMidBrickBit = 0x80u;
  f32x3 sampleRelativeToMidBrick = sampleRelativeToTopBrick;
  f32x3 midBrickPos = f32x3(0.0f);
  if (sampleRelativeToTopBrick.data[0] > 0.5f) {
    sampleRelativeToMidBrick.data[0] -= 0.5f;
    midBrickPos.data[0] = 0.5f;
    checkMidBrickBit >>= 4u;
  }
  if (sampleRelativeToTopBrick.data[1] > 0.5f) {
    sampleRelativeToMidBrick.data[1] -= 0.5f;
    midBrickPos.data[1] = 0.5f;
    checkMidBrickBit >>= 2u;
  }
  if (sampleRelativeToTopBrick.data[2] > 0.5f) {
    sampleRelativeToMidBrick.data[2] -= 0.5f;
    midBrickPos.data[2] = 0.5f;
    checkMidBrickBit >>= 1u;
  }
  if ((midBricks & checkMidBrickBit) == 0u) {
    f32x3 midBrickMin = grid2Min + f32x3(topBrickPos) + midBrickPos;
    f32x3 midBrickMax = midBrickMin + f32x3(0.5f);
    f32x3 p1 = samplePos;
    f32x3 p2 = p1 + sampleDir * 2.0f;
    f32x3 m = p2 - p1;
    float tMin, tMax;
    rayBoxIntersectTime(__ctx, p1, f32x3(1.0f) / m, midBrickMin, midBrickMax,
                        tMin, tMax);
    skipPos = p1 + m * (tMax + 0.01f);
    return false;
  }
  unsigned skipMidBricks =
      countSetBitsBefore(__ctx, midBricks, checkMidBrickBit);
  streamReadPos += (8u * skipMidBricks);
  unsigned finalMidBrick = readBitsSVO2(__ctx, streamReadPos, 8u);
  unsigned checkVoxelBrickBit = 0x80u;
  if (sampleRelativeToMidBrick.data[0] > 0.25f) {
    checkVoxelBrickBit >>= 4u;
  }
  if (sampleRelativeToMidBrick.data[1] > 0.25f) {
    checkVoxelBrickBit >>= 2u;
  }
  if (sampleRelativeToMidBrick.data[2] > 0.25f) {
    checkVoxelBrickBit >>= 1u;
  }
  if ((checkVoxelBrickBit & finalMidBrick) != 0u)
    return true;
  skipPos = samplePos + sampleDir * 0.25f;
  return false;
}
bool traceRaySVO2(Context &__ctx, f32x3 p1, f32x3 p2, f32x3 &hitPos) {
  f32x3 m = p2 - p1;
  float hitMin, hitMax;
  if (!rayBoxIntersectTime(__ctx, p1, f32x3(1.0f) / m, grid2Min, grid2Max,
                           hitMin, hitMax)) {
    hitPos = f32x3(-1.0f);
    return false;
  }
  hitMin += 0.00001f;
  hitMax -= 0.00001f;
  f32x3 curPos = p1 + hitMin * m;
  f32x3 curDir = normalize(m);
  f32x3 skipPos = f32x3(0.0f);
  for (int i = 0; i != 100; i++) {
    if (readLeafSVO2(__ctx, curPos, curDir, skipPos)) {
      hitPos = curPos;
      return true;
    }
    if (skipPos == f32x3(10000.0f))
      break;
    curPos = skipPos;
  }
  return false;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 fetchUV = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x2 uvDenorm = (fetchUV - f32x2(0.5f)) * 2.0f;
  float curTime = iTime * 0.5f;
  f32x3 curEye = f32x3(sin(curTime) * 20.0f, 3.0f, cos(curTime) * 20.0f);
  f32x3 curLook = normalize(-curEye);
  f32x3 curSide = normalize(cross(f32x3(0.0f, 1.0f, 0.0f), curLook));
  f32x3 curUp =
      cross(curLook, curSide) * (iResolution.data[1] / iResolution.data[0]);
  f32x3 curDir =
      curLook + curSide * uvDenorm.data[0] + curUp * uvDenorm.data[1];
  f32x3 curEndSight = curEye + curDir * 1000.0f;
  f32x3 curSightSeg = curEndSight - curEye;
  f32x3 hitPos = f32x3(0.0f);
  bool rt;
  if (mod(iTime, 3.0f) < 1.0f)
    rt = traceRaySVO0(__ctx, curEye, curSightSeg, hitPos);
  else if (mod(iTime, 3.0f) < 2.0f)
    rt = traceRaySVO1(__ctx, curEye, curSightSeg, hitPos);
  else
    rt = traceRaySVO2(__ctx, curEye, curSightSeg, hitPos);
  if (!rt) {
    if (hitPos == f32x3(-1.0f))
      fragColor = f32x4(fetchUV.data[0], fetchUV.data[1], 0.0f, 1.0f);
    else
      fragColor = f32x4(0.0f);
  } else
    fragColor = f32x4(length(hitPos - curEye) * 0.05f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output) {
  iResolution = f32x3(p->resolution_time.data[0], p->resolution_time.data[1],
                      p->resolution_time.data[2]);
  gl_FragCoord =
      f32x4(frag_coord[0], frag_coord[1], frag_coord[2], frag_coord[3]);
  iTime = p->resolution_time.data[3];
  iMouse = f32x4(p->mouse.data[0], p->mouse.data[1], p->mouse.data[2],
                 p->mouse.data[3]);
  iDate =
      f32x4(p->date.data[0], p->date.data[1], p->date.data[2], p->date.data[3]);
  iTimeDelta = p->time_delta;
  iFrameRate = p->frame_rate;
  iFrame = p->frame;
  iSampleRate = p->sample_rate;
  for (unsigned i = 0; i < 4; ++i) {
    iChannelTime[i] = p->channel_time[i];
    iChannelResolution[i] = f32x3(p->channel_resolution[i].data[0],
                                  p->channel_resolution[i].data[1],
                                  p->channel_resolution[i].data[2]);
  }
  Context __ctx{};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
