#ifndef READXL_CELLTYPE_
#define READXL_CELLTYPE_

#include <Rcpp.h>
#include <libxls/xls.h>
#include "StringSet.h"

enum CellType {
  CELL_BLANK,
  CELL_DATE,
  CELL_NUMERIC,
  CELL_TEXT
};

// COL_BLANK is a column full of CELL_BLANKs,
// while COL_SKIP is a column the user-specified type
enum ColType {
  COL_BLANK,
  COL_DATE,
  COL_NUMERIC,
  COL_TEXT,
  COL_LIST,
  COL_SKIP
};

// ColType enum is ordered such that a simple cast
// is sufficient to convert from CellType (user-specified
// columns come after cell types)
ColType inline as_ColType(CellType cell) {
  return (ColType) cell;
}

bool inline isDateTime(int id, const std::set<int> custom);

inline std::vector<ColType> colTypeStrings(Rcpp::CharacterVector x) {
  std::vector<ColType> types;
  types.reserve(x.size());

  for (int i = 0; i < x.size(); ++i) {
    std::string type(x[i]);
    if (type == "blank") {
      types.push_back(COL_BLANK);
    } else if (type == "date") {
      types.push_back(COL_DATE);
    } else if (type == "numeric") {
      types.push_back(COL_NUMERIC);
    } else if (type == "text") {
      types.push_back(COL_TEXT);
    } else if (type == "list") {
      types.push_back(COL_LIST);
    } else if (type == "skip") {
      types.push_back(COL_SKIP);
    } else {
      Rcpp::stop("Unknown type '%s' at position %i", type, i + 1);
    }
  }

  return types;
}

inline std::string colTypeDesc(ColType type) {
  switch(type) {
  case COL_BLANK:   return "blank";
  case COL_DATE:    return "date";
  case COL_NUMERIC: return "numeric";
  case COL_TEXT:    return "text";
  case COL_LIST:    return "list";
  case COL_SKIP:    return "skip";
  }
  return "???";
}

inline CellType cellType(xls::st_cell::st_cell_data cell, xls::st_xf* styles,
                         const std::set<int>& customDateFormats,
                         const StringSet &na = "") {
  // Find codes in [MS-XLS] S2.3.2 (p175).
  // See xls_addCell for those used for cells
  switch(cell.id) {
  case 253: // LabelSst
  case 516: // Label
    return na.contains((char*) cell.str) ? CELL_BLANK : CELL_TEXT;
    break;

  case 6:    // formula
  case 1030: // formula (Apple Numbers Bug)
    if (cell.l == 0) {
      return na.contains(cell.d) ? CELL_BLANK : CELL_NUMERIC;
    } else {
      if (na.contains((char*) cell.str)) {
        return CELL_BLANK;
      } else {
        return CELL_TEXT;
      }
    }
    break;

  case 189: // MulRk
  case 515: // Number
  case 638: // Rk
    {
      if (na.contains(cell.d))
        return CELL_BLANK;

      if (styles == NULL)
        return CELL_NUMERIC;

      int format = styles->xf[cell.xf].format;
      return isDateTime(format, customDateFormats) ? CELL_DATE : CELL_NUMERIC;
    }
    break;

  case 190: // MulBlank
  case 513: // Blank
    return CELL_BLANK;
    break;

  default:
    Rcpp::Rcout << "Unknown type: " << cell.id << "\n";
  return CELL_NUMERIC;
  }
}

bool inline isDateTime(int id, const std::set<int> custom) {
  // Date formats:
  // ECMA-376 (http://www.ecma-international.org/publications/standards/Ecma-376.htm)
  // 18.8.30 numFmt (Number Format)  (p1777)
  // Date times: 14-22, 27-36, 45-47, 50-58, 71-81 (inclusive)
  if ((id >= 14 && id <= 22) ||
    (id >= 27 && id <= 36) ||
    (id >= 45 && id <= 47) ||
    (id >= 50 && id <= 58) ||
    (id >= 71 && id <= 81))
    return true;

  // Built-in format that's not a date
  if (id < 164)
    return false;

  return custom.count(id) > 0;
}

inline bool isDateFormat(std::string x) {
  for (size_t i = 0; i < x.size(); ++i) {
    switch (x[i]) {
    case 'd':
    case 'D':
    case 'm': // 'mm' for minutes
    case 'M':
    case 'y':
    case 'Y':
    case 'h': // 'hh'
    case 'H':
    case 's': // 'ss'
    case 'S':
      return true;
    default:
      break;
    }
  }

  return false;
}

inline Rcpp::RObject makeCol(ColType type, int n) {
  switch(type) {
  case COL_BLANK:
  case COL_SKIP:
    return R_NilValue;
    break;
  case COL_DATE: {
    Rcpp::RObject col = Rcpp::NumericVector(n, NA_REAL);
    col.attr("class") = Rcpp::CharacterVector::create("POSIXct", "POSIXt");
    col.attr("tzone") = "UTC";
    return col;
  }
    break;
  case COL_NUMERIC:
    return Rcpp::NumericVector(n, NA_REAL);
    break;
  case COL_TEXT:
    return Rcpp::CharacterVector(n, NA_STRING);
    break;
  case COL_LIST:
    return Rcpp::List(n, Rcpp::LogicalVector(1, NA_LOGICAL));
    break;
  }

  return R_NilValue;
}

inline Rcpp::List removeSkippedColumns(Rcpp::List cols,
                                       Rcpp::CharacterVector names,
                                       std::vector<ColType> types) {
  int p = cols.size();

  int p_out = 0;
  for (int j = 0; j < p; ++j) {
    if (types[j] != COL_SKIP)
      p_out++;
  }

  Rcpp::List out(p_out);
  Rcpp::CharacterVector names_out(p_out);
  int j_out = 0;
  for (int j = 0; j < p; ++j) {
    if (types[j] == COL_SKIP) {
      continue;
    }

    out[j_out] = cols[j];
    names_out[j_out] = names[j];
    j_out++;
  }

  // Turn list into a data frame
  out.attr("names") = names_out;

  return out;
}

#endif
