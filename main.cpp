#include <memory>
#include <vector>
#include <iostream>
#include <optional>
#include <version>
#include <chrono>
#include <sys/time.h>
#include <string>
#include <strstream>
#include <fmt/chrono.h>
#include <fmt/ostream.h>
#include <algorithm>
#include <cstdint>

using namespace std;
// #ifdef __has_cpp_attribute(__cpp_lib_bit_cast)
// int testtest_11()
// {
//   return 17;
// }
// #endif
#ifdef __cpp_lib_bit_cast
int testtest_11b()
{
  return 17;
}
#endif
#ifdef __cpp_capture_star_this
int testtest_22()
{
  return 17;
}
#endif

typedef uint64_t rep;
typedef std::chrono::duration<rep, std::nano> timespan;
using namespace chrono_literals;

std::string timespan_str(timespan t)
{
  // FIXME: somebody pretty please make a version of this function
  // that isn't as lame as this one!
  uint64_t nsec = std::chrono::nanoseconds(t).count();
  std::ostringstream ss;
  if (nsec < 2000000000) {
    ss << ((float)nsec / 1000000000) << "s";
    return ss.str();
  }
  uint64_t sec = nsec / 1000000000;
  if (sec < 120) {
    ss << sec << "s";
    return ss.str();
  }
  uint64_t min = sec / 60;
  if (min < 120) {
    ss << min << "m";
    return ss.str();
  }
  uint64_t hr = min / 60;
  if (hr < 48) {
    ss << hr << "h";
    return ss.str();
  }
  uint64_t day = hr / 24;
  if (day < 14) {
    ss << day << "d";
    return ss.str();
  }
  uint64_t wk = day / 7;
  if (wk < 12) {
    ss << wk << "w";
    return ss.str();
  }
  uint64_t mn = day / 30;
  if (mn < 24) {
    ss << mn << "M";
    return ss.str();
  }
  uint64_t yr = day / 365;
  ss << yr << "y";
  return ss.str();
}

std::string exact_timespan_str(timespan t)
{
  uint64_t nsec = std::chrono::nanoseconds(t).count();
  uint64_t sec = nsec / 1000000000;
  nsec %= 1'000'000'000;
  uint64_t yr = sec / (60 * 60 * 24 * 365);

  //cout << "\n===== " << std::chrono::nanoseconds(t).count() << " x " << nsec <<
  //   " -> s " << sec << "\n";
  std::ostringstream ss;
  if (yr) {
    ss << yr << "y";
    sec -= yr * (60 * 60 * 24 * 365);
  }
  uint64_t mn = sec / (60 * 60 * 24 * 30);
  if (mn >= 3) {
    ss << mn << "mo";
    sec -= mn * (60 * 60 * 24 * 30);
  }
  uint64_t wk = sec / (60 * 60 * 24 * 7);
  if (wk >= 2) {
    ss << wk << "w";
    sec -= wk * (60 * 60 * 24 * 7);
  }
  uint64_t day = sec / (60 * 60 * 24);
  if (day >= 2) {
    ss << day << "d";
    sec -= day * (60 * 60 * 24);
  }
  uint64_t hr = sec / (60 * 60);
  if (hr >= 2) {
    ss << hr << "h";
    sec -= hr * (60 * 60);
  }
  uint64_t min = sec / 60;
  if (min >= 2) {
    ss << min << "m";
    sec -= min * 60;
  }
  if (sec || nsec) {
    // ss << sec;

    if (nsec) {
      ss << (((float)nsec / 1'000'000'000) + sec) << "s";
    } else {
      ss << sec << "s";
    }
  }
  //if (sec || nsec) {
  //  ss << "s";
  //}
  return ss.str();
}
using std::chrono::seconds;
using namespace std;

using cmsec = std::chrono::duration<int64_t, std::milli>;
static std::vector<cmsec/*std::chrono::seconds*/> format_limits {

   /* show years */
  // 'years' require c++20... std::chrono::duration_cast<seconds>(chrono::years{1}),
  31556952s,

  // show months (from 3 months on)
  cmsec(1000ll* 3 * 2'629'746),

  // show weeks (do we really want to?)
  cmsec(1000ll* 2 * 604'800),

  // show days (2 days and up)
  cmsec(1000* 2 * 86'400),

  // show hours (2+)
  cmsec(1000* 2 * 3'600),

  cmsec(1000* 2 * 60),

  cmsec(2000),

  cmsec(0)
};

static std::vector<string> selected_f_name {

  "0-YMD",
  "1-MWD",
  "2-KDH",
  "3-DHm",
  "4-Hms",
  "5-Msl?",
  "6-sl?",
  "7-l"
};


static std::vector<string> selected_fmt {

  "0: {1:}y{2:}y{4:}d",
  "1: {2:}m{3:}w{4:}Diw",
  "2: {3:}w{4:}Diw{7:}H",
  "3: {6:}D{7:02}H{9:02}M",
  "4: {8:}H{9:02}M{11:02}S",
  "5: {10}M{11:02}S{0:03}ms",
  "6: {12:}S{0:03}ms",
  "7: {0:}ms"
};


string fmt_byrange(cmsec x)
{
  auto fnt = find_if(format_limits.begin(), format_limits.end(), [x](cmsec from_tbl){ return x >= from_tbl; });
  return selected_fmt[fnt-format_limits.begin()];
}


string dump_str(cmsec x)
{
  auto a = duration_cast<std::chrono::duration<int64_t>>(x).count();
  auto just_ms = x.count() - a * 1'000ll;

  /* param 1: years   */                auto p1_years = a / 3'155'695'2ll;
  int64_t ma = a % 3'155'695'2ll;
  /* param 2: months  */                auto p2_months = ma / 2'629'746;
  int64_t ma_wM = ma % 2'629'746;
  /* param 3: weeks (imply months) */   auto p3_weeks = ma_wM / 604'800;
  /* param 4: days, if months  */       auto p4_days_wM = ma_wM / 86'400;
  /* param 5: days, if weeks  */        auto p5_days_wWeeks = (ma_wM % 604'800) / 86'400;
  /* param 6: days, w/o M, Wk  */       auto p6_days = ma / 86'400;

  int64_t md = ma % 86'400;
  /* param 7: H, if D  */               auto p7_H_wD = md / 3'600;
  /* param 8: H, w/o D  */              auto p8_H = ma / 3'600;

  /* param 9: M, if H  */               auto p9_M_wH = (ma - p8_H*60) % 60;
  /* param 10: M, w/o H  */             auto p10_M = ma % 60;

  // assuming for p11 that there is nothing higher than minutes
  /* param 11: s, if M  */              auto p11_S_wM = (ma - p10_M*60) % 60;
  /* param 12: s, w/o H  */             auto p12_S = ma % 60;


  cout << fmt::format("Selected the following format: {}\n", fmt_byrange(x));

//  for (const auto& ff : selected_fmt) {
//
//    cout << fmt::format(ff, p1_years, p2_months, p3_weeks, p4_days_wM, p5_days_wWeeks, p6_days, p7_H_wD, p8_H, p9_M_wH, p10_M, 111, 112, 113, 114, 115) << " --- \n";
//  }

  auto res = fmt::format(fmt_byrange(x), just_ms, p1_years, p2_months, p3_weeks, p4_days_wM, p5_days_wWeeks, p6_days, p7_H_wD, p8_H, p9_M_wH, p10_M, p11_S_wM, p12_S, 999);

  return res;
}







string byrange(int64_t msecs)
{
  auto x = cmsec{msecs};

  cout << fmt::format("-- byrange {} -> {}\n", msecs, x);

  auto fnt = find_if(format_limits.begin(), format_limits.end(), [x](cmsec from_tbl){ return x >= from_tbl; });

  cout << fmt::format("-- byrange {} -> {}  found {}\n", msecs, x, *fnt);

  return selected_f_name[fnt-format_limits.begin()];

}


void basic_vec_test()
{
  for (auto r : format_limits) {
    cout << fmt::format("{}  {}\n", r.count(), r);
  }

   cout << " for 8000s? " << byrange(8'000'000) << "\n";

   cout << " for 4 months, 3 d? " << dump_str( cmsec{4000ll * 2629746ll + 3000ll * 86400ll}) << "\n";;
   cout << " for 5d, 1h " << dump_str( cmsec{ 5000ll * 86400ll + 3'600'000}) << "\n";;
}


// some test cases:
// in y,m,d,...ms and expected result

struct fmtcase_t {

int y_;
int m_;
int d_;

int h_;
int min_;
int s_;

int ms_;

string exp_;

cmsec to_cmsec() {

  return cmsec{ms_ + 1000ll*(31556952ll*y_ + 2629746ll*m_ + 86400ll*d_ + 3600*h_ + 60*min_ + s_)};
};
};


vector<fmtcase_t> fmtcases {
  fmtcase_t{ 0, 0, 0,   0, 0, 0, 777, "0.777s" },
  fmtcase_t{ 0, 0, 0,   0, 0, 1, 777, "1.777s" },
  fmtcase_t{ 0, 0, 0,   0, 1, 1, 777, "1M1.777s" },

  fmtcase_t{ 0, 0, 1,   0, 0, 0, 777, "maybe 24H w/o the rest?" },
  fmtcase_t{ 0, 0, 1,   0, 0, 1, 777, "1.777s" },
  fmtcase_t{ 0, 0, 1,   0, 1, 1, 777, "24H01M01S" }
  , fmtcase_t{ 0, 0, 3,   0, 1, 1, 777, "3D00H01M" }
  , fmtcase_t{ 0, 0, 3,   0, 1, 1, 0, "v" }
  , fmtcase_t{ 0, 3, 3,   0, 1, 1, 777, "v" }
  , fmtcase_t{ 0, 1, 3,   0, 1, 1, 777, "v" }

};


void test_fmtcases()
{
  for (auto& f : fmtcases) {

    auto f_as_sms{f.to_cmsec()};
    cout << fmt::format(" test case <{}/{}/{} {}/{}/{} {}> ({} ? ?)  ---> {}\n",
       f.y_, f.m_, f.d_, f.h_, f.min_, f.s_, f.ms_, f.exp_, dump_str(f_as_sms));
  }

}

std::string suggested(timespan t)
{


  uint64_t nsec = std::chrono::nanoseconds(t).count();
  uint64_t sec = nsec / 1000000000;
  nsec %= 1'000'000'000;
  uint64_t yr = sec / (60 * 60 * 24 * 365);

  //cout << "\n===== " << std::chrono::nanoseconds(t).count() << " x " << nsec <<
  //   " -> s " << sec << "\n";
  std::ostringstream ss;
  if (yr) {
    ss << yr << "y";
    sec -= yr * (60 * 60 * 24 * 365);
  }
  uint64_t mn = sec / (60 * 60 * 24 * 30);
  if (mn >= 3) {
    ss << mn << "mo";
    sec -= mn * (60 * 60 * 24 * 30);
  }
  uint64_t wk = sec / (60 * 60 * 24 * 7);
  if (wk >= 2) {
    ss << wk << "w";
    sec -= wk * (60 * 60 * 24 * 7);
  }
  uint64_t day = sec / (60 * 60 * 24);
  if (day >= 2) {
    ss << day << "d";
    sec -= day * (60 * 60 * 24);
  }
  uint64_t hr = sec / (60 * 60);
  if (hr >= 2) {
    ss << hr << "h";
    sec -= hr * (60 * 60);
  }
  uint64_t min = sec / 60;
  if (min >= 2) {
    ss << min << "m";
    sec -= min * 60;
  }
  if (sec || nsec) {
    // ss << sec;

    if (nsec) {
      ss << (((float)nsec / 1'000'000'000) + sec) << "s";
    } else {
      ss << sec << "s";
    }
  }
  //if (sec || nsec) {
  //  ss << "s";
  //}
  return ss.str();
}


std::string orig(timespan t)
{
  uint64_t nsec = std::chrono::nanoseconds(t).count();
  uint64_t sec = nsec / 1000000000;
  nsec %= 1000000000;
  uint64_t yr = sec / (60 * 60 * 24 * 365);
  std::ostringstream ss;
  if (yr) {
    ss << yr << "y";
    sec -= yr * (60 * 60 * 24 * 365);
  }
  uint64_t mn = sec / (60 * 60 * 24 * 30);
  if (mn >= 3) {
    ss << mn << "mo";
    sec -= mn * (60 * 60 * 24 * 30);
  }
  uint64_t wk = sec / (60 * 60 * 24 * 7);
  if (wk >= 2) {
    ss << wk << "w";
    sec -= wk * (60 * 60 * 24 * 7);
  }
  uint64_t day = sec / (60 * 60 * 24);
  if (day >= 2) {
    ss << day << "d";
    sec -= day * (60 * 60 * 24);
  }
  uint64_t hr = sec / (60 * 60);
  if (hr >= 2) {
    ss << hr << "h";
    sec -= hr * (60 * 60);
  }
  uint64_t min = sec / 60;
  if (min >= 2) {
    ss << min << "m";
    sec -= min * 60;
  }
  if (sec) {
    ss << sec;
  }
  if (nsec) {
    ss << ((float)nsec / 1000000000);
  }
  if (sec || nsec) {
    ss << "s";
  }
  return ss.str();
}


string test_tm(int s)
{
  return timespan_str(timespan{std::chrono::seconds{s}}) + " --\t"s +
         exact_timespan_str(timespan{std::chrono::milliseconds{1000*s + 567}}) +
         "\t\t"s +
         exact_timespan_str(timespan{std::chrono::milliseconds{1000*s}}) +
         "\t\t"s +
         orig(timespan{std::chrono::milliseconds{1000*s}}) +
         " % \t"s +
         orig(timespan{std::chrono::milliseconds{1000*s + 567}});

}

void test_times()
{
  cout << fmt::format("3600:         {}\n", test_tm(3600));
  cout << fmt::format("1d:           {}\n", test_tm(3600*24));
  cout << fmt::format("2d2h:         {}\n", test_tm(3600*50));
  cout << fmt::format("2d2h2s:       {}\n", test_tm(3600*50+2));
  cout << fmt::format("17:           {}\n", test_tm(17));
  cout << fmt::format("67:           {}\n", test_tm(67));
  cout << fmt::format("170:          {}\n", test_tm(170));
  cout << fmt::format("3670:         {}\n", test_tm(3670));
  //cout << fmt::format("43200+90:     {}\n", test_tm(43200+90));
  //cout << fmt::format("1d+6h+90s:    {}\n", test_tm(3*43200+90));

}

uint8_t shachar(uint8_t a, uint8_t b) { return a+b; }

std::vector<const char*>
argv_to_vec(int ac, const char * const *av)
{

  return {av+1, av+ac};
}

// std::vector<const char*> f1(int ac, const char** av)
// {
//   cout << "\n" << __func__ << "\n";
//   std::vector<const char*> args;
//   argv_to_vec(ac, av, args);
//   return args;
// }

std::vector<const char*> f2(int ac, const char** av)
{
  cout << "\n" << __func__ << "\n";
  return argv_to_vec(ac, av);
}

std::vector<const char*> f4(int ac, char** av)
{
  cout << "\n" << __func__ << "\n";
  return argv_to_vec(ac, av);
}

// std::vector<const char*> f2(int ac, char** av)
// {
//   cout << "\n" << __func__ << "\n";
//   return *argv_to_opt_vec(ac, av);
// }

void pp(const std::vector<const char*>& v)
{
  for (auto x : v) {

    cout << "-- " << x << "\n";

  }
}


int main(int ac, const char** av)
{
  test_fmtcases();
  basic_vec_test();

/*
  //pp(f1(ac, av));
  pp(argv_to_vec(ac, av));

  pp(f2(ac, av));
  //auto v1 = argv_to_opt_vec(ac, av);


  test_times();
  */

}