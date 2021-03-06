context("Column types")

test_that("illegal col_types are rejected", {
  expect_error(
    read_excel(test_sheet("types.xlsx"),
               col_types = c("foo", "numeric", "text", "bar")),
    "Illegal column type"
  )
})

test_that("request for 'blank' col type gets deprecation message and fix", {
  expect_message(
    read_excel(test_sheet("types.xlsx"),
               col_types = rep_len(c("blank", "text"), length.out = 5)),
    "`col_type = \"blank\"` deprecated. Use \"skip\" instead.",
    fixed = TRUE
  )
})

test_that("invalid col_types are rejected", {
  expect_error(
    read_excel(test_sheet("types.xlsx"), col_types = character()),
    "length(col_types) > 0 is not TRUE", fixed = TRUE
  )
  expect_error(
    read_excel(test_sheet("types.xlsx"), col_types = 1:3),
    "is.character(col_types) is not TRUE", fixed = TRUE
  )
  expect_error(
    read_excel(test_sheet("types.xlsx"), col_types = c(NA, "text", "numeric")),
    "!anyNA(col_types) is not TRUE", fixed = TRUE
  )
})

test_that("col_types can be specified", {
  df <- read_excel(test_sheet("iris-excel.xlsx"),
                   col_types = c("numeric", "text", "numeric", "numeric", "text"))
  expect_is(df[[2]], "character")
})

test_that("col_types are recycled", {
  df <- read_excel(test_sheet("types.xlsx"), col_types = "text")
  expect_match(vapply(df, class, character(1)), "character")
})

test_that("inappropriate col_types generate warning", {
  expect_warning(
    read_excel(test_sheet("iris-excel.xlsx"),
               col_types = c("numeric", "text", "numeric", "numeric", "numeric")),
    "expecting numeric"
  )
})

test_that("types imputed & read correctly [xlsx]", {
  types <- read_excel(test_sheet("types.xlsx"))
  expect_is(types$number, "numeric")
  expect_is(types$string, "character")
  expect_is(types$boolean, "numeric")
  expect_is(types$date, "POSIXct")
  expect_is(types$string_in_row_3, "character")
  skip("switch expecation to logical (vs numeric) when possible")
})

test_that("types imputed & read correctly [xls]", {
  expect_output(
    ## valgrind reports this
    ## Conditional jump or move depends on uninitialised value(s)
    types <- read_excel(test_sheet("types.xls")),
    "Unknown type: 517"
    ## definitely due to these 'Unknown type: 517' msgs
    ## line 102 in CellType.h
    ##   Rcpp::Rcout << "Unknown type: " << cell.id << "\n";
    ## if I skip this test, memcheck report is as clean as it ever gets
    ## https://github.com/tidyverse/readxl/issues/259
  )
  expect_is(types$number, "numeric")
  expect_is(types$string, "character")
  #expect_is(types$boolean, "numeric")
  #expect_is(types$date, "POSIXct")
  expect_is(types$string_in_row_3, "character")
  skip("revisit these expectations as xls problems are fixed")
})

test_that("guess_max is honored for col_types [xlsx]", {
  expect_warning(
    types <- read_excel(test_sheet("types.xlsx"), guess_max = 2),
    "expecting numeric"
  )
  expect_identical(types$string_in_row_3, c(1, 2, NA))
})

test_that("guess_max is honored for col_types [xls]", {
  skip("write this test as xls problems are fixed")
})

test_that("wrong length col types generates error", {
  expect_error(
    read_excel(test_sheet("iris-excel.xlsx"), col_types = c("numeric", "text")),
    "Sheet 1 has 5 columns, but `col_types` has length 2."
  )
  expect_error(
    read_excel(test_sheet("iris-excel.xls"), col_types = c("numeric", "text")),
    "Received 5 names but 2 types."
  )
})

test_that("list column reads data correctly [xlsx]", {
  types <- read_excel(test_sheet("list_type.xlsx"), col_types = "list")
  expect_equal(types$var1[[1]], 1)
  expect_equal(types$var1[[2]], NA)
  expect_equal(types$var1[[3]], "a")
  expect_equal(types$var1[[4]], as.POSIXct("2017-01-01", tz = "UTC"))
  expect_equal(types$var1[[5]], "abc")
})

test_that("setting `na` works in list columns [xlsx]", {
  na_defined <-  read_excel(test_sheet("list_type.xlsx"), col_types = "list", na = "a")
  expect_equal(na_defined$var1[[3]], NA)
})

test_that("list column reads data correctly [xls]", {
  types <- read_excel(test_sheet("list_type.xls"), col_types = "list")
  expect_equal(types$var1[[1]], 1)
  expect_equal(types$var1[[2]], NA)
  expect_equal(types$var1[[3]], "a")
  expect_equal(types$var1[[4]], as.POSIXct("2017-01-01", tz = "UTC"))
  expect_equal(types$var1[[5]], "abc")
})

test_that("setting `na` works in list columns [xls]", {
  na_defined <-  read_excel(test_sheet("list_type.xls"), col_types = "list", na = "a")
  expect_equal(na_defined$var1[[3]], NA)
})
