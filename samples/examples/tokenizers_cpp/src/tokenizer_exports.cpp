// @File: StaticExports.cc
//
// Purpose:
//  This file contains the implementation of a function that loads a tokenizer from a JSON blob
//
//**************************************************************************************************

#include "tokenizers_cpp.h"

//--------------------------------------------------------------------------------------------------
// Name: LoadBlobJsonAndEncode
//
// Description:
//  Exports a function for use in other modules or applications.
//  Loads a tokenizer from a JSON blob and encodes the input text into token IDs.
// Parameters:
//  json_blob - The serialized tokenizer JSON.
//  text      - The input string to tokenize.
//  token_ids - Output vector to receive the encoded token IDs.
//
extern "C" __declspec(dllexport) void LoadBlobJsonAndEncode(const std::string& json_blob, const std::string& text, std::vector<int32_t>& token_ids)
{
  auto tokenizer = tokenizers::Tokenizer::FromBlobJSON(json_blob);
  token_ids = tokenizer->Encode(text);
}
