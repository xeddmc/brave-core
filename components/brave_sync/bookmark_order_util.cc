/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_sync/bookmark_order_util.h"

#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"

namespace brave_sync {

std::vector<int> OrderToIntVect(const std::string& s) {
  std::vector<std::string> vec_s = SplitString(
      s,
      ".",
      base::WhitespaceHandling::TRIM_WHITESPACE,
      base::SplitResult::SPLIT_WANT_NONEMPTY);
  std::vector<int> vec_int;
  vec_int.reserve(vec_s.size());
  for (size_t i = 0; i < vec_s.size(); ++i) {
    int output = 0;
    bool b = base::StringToInt(vec_s[i], &output);
    CHECK(b);
    CHECK(output >= 0);
    vec_int.emplace_back(output);
  }
  return vec_int;
}

std::string ToOrderString(const std::vector<int> &vec_int) {
  std::string ret;
  for (size_t i = 0; i < vec_int.size(); ++i) {
    if (vec_int[i] < 0) {
      return "";
    }
    ret += std::to_string(vec_int[i]);
    if (i != vec_int.size() - 1) {
      ret += ".";
    }
  }
  return ret;
}

bool CompareOrder(const std::string& left, const std::string& right) {
  // Return: true if left <  right
  // Split each and use C++ stdlib
  std::vector<int> vec_left = OrderToIntVect(left);
  std::vector<int> vec_right = OrderToIntVect(right);

  return std::lexicographical_compare(vec_left.begin(), vec_left.end(),
    vec_right.begin(), vec_right.end());
}

namespace {

std::string GetNextOrderFromPrevOrder(std::vector<int> &vec_int) {
  DCHECK(vec_int.size() > 2);
  int last_number = vec_int[vec_int.size() - 1];
  DCHECK_GT(last_number, 0);
  if (last_number <= 0) {
    return "";
  } else {
    vec_int[vec_int.size() - 1]++;
    return ToOrderString(vec_int);
  }
}


std::string GetPrevOrderFromNextOrder(int last_number,
    std::vector<int> &vec_result) {
  DCHECK_GT(last_number, 0);
  if (last_number <= 0) {
    return "";
  } else if (last_number == 1) {
    return ToOrderString(vec_result) + ".0.1";
  } else {
    vec_result.push_back(last_number - 1);
    return ToOrderString(vec_result);
  }
}

std::string GetPrevOrderFromNextOrder(std::vector<int> &vec_int) {
  DCHECK(vec_int.size() > 2);
  int last_number = vec_int[vec_int.size() - 1];
  DCHECK_GT(last_number, 0);
  vec_int.resize(vec_int.size() - 1);
  return GetPrevOrderFromNextOrder(last_number, vec_int);
}

}

// Ported from https://github.com/brave/sync/blob/staging/client/bookmarkUtil.js
std::string GetOrder(const std::string& prev, const std::string& next,
    const std::string& parent) {
  if (prev.empty() && next.empty()) {
    return parent + ".1";
  } else if (!prev.empty() && next.empty()) {
    std::vector<int> vec = OrderToIntVect(prev);
    return GetNextOrderFromPrevOrder(vec);
  } else if (prev.empty() && !next.empty()) {
    std::vector<int>  vec = OrderToIntVect(next);
    DCHECK(vec.size() > 2);
    return GetPrevOrderFromNextOrder(vec);
  } else {
    DCHECK(!prev.empty() && !next.empty());
    std::vector<int> vec_prev = OrderToIntVect(prev);
    DCHECK(vec_prev.size() > 2);
    std::vector<int> vec_next = OrderToIntVect(next);
    DCHECK(vec_next.size() > 2);

    if (vec_prev.size() == vec_next.size()) {
      // Orders have the same length
      if (vec_next[vec_next.size() - 1] - vec_prev[vec_prev.size() - 1] > 1) {
        vec_prev[vec_prev.size() - 1]++;
        return ToOrderString(vec_prev);
      } else {
        return prev + ".1";
      }
    } else if (vec_next.size() > vec_prev.size()) {
      // Next order is longer than previous order
      // |prev_numbers_equal_to_next| means prev numbers except the last number
      // are equal to corresponding next numbers
      bool prev_numbers_equal_to_next = true;
      for (size_t i = 0; i < vec_prev.size() - 1; ++i) {
        if (vec_prev[i] != vec_next[i]) {
          prev_numbers_equal_to_next = false;
          break;
        }
      }

      // result will be based on prev
      std::vector<int> vec_result = vec_prev;

      // If next has zeros beyond prev size, copy them to result
      int current_index = vec_prev.size();
      while (vec_next[current_index] == 0) {
        vec_result.push_back(0);
        current_index++;
      }

      int last_number_next = vec_next[current_index];
      int last_number_prev = vec_prev[vec_prev.size() - 1];

      if (prev_numbers_equal_to_next) {
        int same_position_number_next = vec_next[vec_prev.size() - 1];
        if ((same_position_number_next - last_number_prev) >= 1) {
          vec_result.push_back(1);
          return ToOrderString(vec_result);
        } else {
          auto vresult = GetPrevOrderFromNextOrder(
              last_number_next, vec_result);
          return vresult;
        }
      } else {
        // In the worst case at current point:
        // prev is a.b.c.d.e
        // next is a.b.c.x.y.z
        // 1) At least two right digits in prev are different because
        // prev_numbers_equal_to_next==false
        // 2) (prev) < (next) and (1) mean
        //     first_diff_digit_of_prev < first_diff_digit_of_next
        // So can produce result as
        //     a.b.c.d.e+1
        std::vector<int> vec = OrderToIntVect(prev);
        return GetNextOrderFromPrevOrder(vec);
      }
    } else {
      // Prev order is longer than next order
      DCHECK(vec_prev.size() > vec_next.size());
      return GetNextOrderFromPrevOrder(vec_prev);
    }
  }

  return "";
}

} // namespace brave_sync
