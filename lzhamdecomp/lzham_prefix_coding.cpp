// File: lzham_prefix_coding.cpp
// LZHAM is in the Public Domain. Please see the Public Domain declaration at the end of include/lzham.h
#include "lzham_core.h"
#include "lzham_prefix_coding.h"

#ifdef LZHAM_BUILD_DEBUG
   //#define TEST_DECODER_TABLES
#endif

namespace lzham
{
   namespace prefix_coding
   {
      bool limit_max_code_size(uint num_syms, uint8* pCodesizes, uint max_code_size)
      {
         const uint cMaxEverCodeSize = 34;            
         
         if ((!num_syms) || (num_syms > cMaxSupportedSyms) || (max_code_size < 1) || (max_code_size > cMaxEverCodeSize))
            return false;
         
         uint num_codes[cMaxEverCodeSize + 1];
         utils::zero_object(num_codes);

         bool should_limit = false;
         
         for (uint i = 0; i < num_syms; i++)
         {
            uint c = pCodesizes[i];
            
            LZHAM_ASSERT(c <= cMaxEverCodeSize);
            
            num_codes[c]++;
            if (c > max_code_size)
               should_limit = true;
         }
         
         if (!should_limit)
            return true;
         
         uint ofs = 0;
         uint next_sorted_ofs[cMaxEverCodeSize + 1];
         for (uint i = 1; i <= cMaxEverCodeSize; i++)
         {
            next_sorted_ofs[i] = ofs;
            ofs += num_codes[i];
         }
            
         if ((ofs < 2) || (ofs > cMaxSupportedSyms))
            return true;
         
         if (ofs > (1U << max_code_size))
            return false;
                           
         for (uint i = max_code_size + 1; i <= cMaxEverCodeSize; i++)
            num_codes[max_code_size] += num_codes[i];
         
         // Technique of adjusting tree to enforce maximum code size from LHArc. 
			// (If you remember what LHArc was, you've been doing this for a LONG time.)
         
         uint total = 0;
         for (uint i = max_code_size; i; --i)
            total += (num_codes[i] << (max_code_size - i));

         if (total == (1U << max_code_size))  
            return true;
            
         do
         {
            num_codes[max_code_size]--;

            uint i;
            for (i = max_code_size - 1; i; --i)
            {
               if (!num_codes[i])
                  continue;
               num_codes[i]--;          
               num_codes[i + 1] += 2;   
               break;
            }
            if (!i)
               return false;

            total--;   
         } while (total != (1U << max_code_size));
         
         uint8 new_codesizes[cMaxSupportedSyms];
         uint8* p = new_codesizes;
         for (uint i = 1; i <= max_code_size; i++)
         {
            uint n = num_codes[i];
            if (n)
            {
               memset(p, i, n);
               p += n;
            }
         }
                                             
         for (uint i = 0; i < num_syms; i++)
         {
            const uint c = pCodesizes[i];
            if (c)
            {
               uint next_ofs = next_sorted_ofs[c];
               next_sorted_ofs[c] = next_ofs + 1;
            
               pCodesizes[i] = static_cast<uint8>(new_codesizes[next_ofs]);
            }
         }
            
         return true;
      }
            
      bool generate_codes(uint num_syms, const uint8* pCodesizes, uint16* pCodes)
      {
         uint num_codes[cMaxExpectedCodeSize + 1];
         utils::zero_object(num_codes);

         for (uint i = 0; i < num_syms; i++)
         {
            uint c = pCodesizes[i];
            LZHAM_ASSERT(c <= cMaxExpectedCodeSize);
            num_codes[c]++;
         }

         uint code = 0;

         uint next_code[cMaxExpectedCodeSize + 1];
         next_code[0] = 0;
         
         for (uint i = 1; i <= cMaxExpectedCodeSize; i++)
         {
            next_code[i] = code;
            
            code = (code + num_codes[i]) << 1;
         }

         if (code != (1 << (cMaxExpectedCodeSize + 1)))
         {
            uint t = 0;
            for (uint i = 1; i <= cMaxExpectedCodeSize; i++)
            {
               t += num_codes[i];
               if (t > 1)
                  return false;
            }
         }

         for (uint i = 0; i < num_syms; i++)
         {
            uint c = pCodesizes[i];
            
            LZHAM_ASSERT(!c || (next_code[c] <= LZHAM_UINT16_MAX));
            
            pCodes[i] = static_cast<uint16>(next_code[c]++);
            
            LZHAM_ASSERT(!c || (math::total_bits(pCodes[i]) <= pCodesizes[i]));
         }
         
         return true;
      }
            
      bool generate_decoder_tables(uint num_syms, const uint8* pCodesizes, decoder_tables* pTables, uint table_bits)
      {
         uint min_codes[cMaxExpectedCodeSize];
         
         if ((!num_syms) || (table_bits > cMaxTableBits))
            return false;
            
         pTables->m_num_syms = num_syms;
         
         uint num_codes[cMaxExpectedCodeSize + 1];
         utils::zero_object(num_codes);

         for (uint i = 0; i < num_syms; i++)
         {
            uint c = pCodesizes[i];
            num_codes[c]++;
         }

         uint sorted_positions[cMaxExpectedCodeSize + 1];
               
         uint next_code = 0;

         uint total_used_syms = 0;
         uint max_code_size = 0;
         uint min_code_size = UINT_MAX;
         for (uint i = 1; i <= cMaxExpectedCodeSize; i++)
         {
            const uint n = num_codes[i];
            
            if (!n)
               pTables->m_max_codes[i - 1] = 0;//UINT_MAX;
            else
            {
               min_code_size = math::minimum(min_code_size, i);
               max_code_size = math::maximum(max_code_size, i);
                  
               min_codes[i - 1] = next_code;
               
               pTables->m_max_codes[i - 1] = next_code + n - 1;
               pTables->m_max_codes[i - 1] = 1 + ((pTables->m_max_codes[i - 1] << (16 - i)) | ((1 << (16 - i)) - 1));
               
               pTables->m_val_ptrs[i - 1] = total_used_syms;
               
               sorted_positions[i] = total_used_syms;
               
               next_code += n;
               total_used_syms += n;
            }

            next_code <<= 1;
         }
         
         pTables->m_total_used_syms = total_used_syms;

         if (total_used_syms > pTables->m_cur_sorted_symbol_order_size)
         {
            pTables->m_cur_sorted_symbol_order_size = total_used_syms;
            
            if (!math::is_power_of_2(total_used_syms))
               pTables->m_cur_sorted_symbol_order_size = math::minimum<uint>(num_syms, math::next_pow2(total_used_syms));
            
            if (pTables->m_sorted_symbol_order)
            {
               lzham_delete_array(pTables->m_sorted_symbol_order);
               pTables->m_sorted_symbol_order = NULL;
            }
            
            pTables->m_sorted_symbol_order = lzham_new_array<uint16>(pTables->m_cur_sorted_symbol_order_size);
            if (!pTables->m_sorted_symbol_order)
               return false;
         }
         
         pTables->m_min_code_size = static_cast<uint8>(min_code_size);
         pTables->m_max_code_size = static_cast<uint8>(max_code_size);
                  
         for (uint i = 0; i < num_syms; i++)
         {
            uint c = pCodesizes[i];
            if (c)
            {
               LZHAM_ASSERT(num_codes[c]);
               
               uint sorted_pos = sorted_positions[c]++;
               
               LZHAM_ASSERT(sorted_pos < total_used_syms);
               
               pTables->m_sorted_symbol_order[sorted_pos] = static_cast<uint16>(i);
            }            
         }

         if (table_bits <= pTables->m_min_code_size)
            table_bits = 0;                                       
         pTables->m_table_bits = table_bits;
                  
         if (table_bits)
         {
            uint table_size = 1 << table_bits;
            if (table_size > pTables->m_cur_lookup_size)
            {
               pTables->m_cur_lookup_size = table_size;
               
               if (pTables->m_lookup)
               {
                  lzham_delete_array(pTables->m_lookup);
                  pTables->m_lookup = NULL;
               }
                  
               pTables->m_lookup = lzham_new_array<uint32>(table_size);
               if (!pTables->m_lookup)
                  return false;
            }
                        
            memset(pTables->m_lookup, 0xFF, static_cast<uint>(sizeof(pTables->m_lookup[0])) * (1UL << table_bits));
            
            for (uint codesize = 1; codesize <= table_bits; codesize++)
            {
               if (!num_codes[codesize])
                  continue;
               
               const uint fillsize = table_bits - codesize;
               const uint fillnum = 1 << fillsize;
               
               const uint min_code = min_codes[codesize - 1];
               const uint max_code = pTables->get_unshifted_max_code(codesize);
               const uint val_ptr = pTables->m_val_ptrs[codesize - 1];
                      
               for (uint code = min_code; code <= max_code; code++)
               {
                  const uint sym_index = pTables->m_sorted_symbol_order[ val_ptr + code - min_code ];
                  LZHAM_ASSERT( pCodesizes[sym_index] == codesize );
                  
                  for (uint j = 0; j < fillnum; j++)
                  {
                     const uint t = j + (code << fillsize);
                     
                     LZHAM_ASSERT(t < (1U << table_bits));
                     
                     LZHAM_ASSERT(pTables->m_lookup[t] == LZHAM_UINT32_MAX);
                     
                     pTables->m_lookup[t] = sym_index | (codesize << 16U);
                  }
               }
            }
         }         
         
         for (uint i = 0; i < cMaxExpectedCodeSize; i++)
            pTables->m_val_ptrs[i] -= min_codes[i];
         
         pTables->m_table_max_code = 0;
         pTables->m_decode_start_code_size = pTables->m_min_code_size;

         if (table_bits)
         {
            uint i;
            for (i = table_bits; i >= 1; i--)
            {
               if (num_codes[i])
               {
                  pTables->m_table_max_code = pTables->m_max_codes[i - 1];
                  break;
               }
            }
            if (i >= 1)
            {
               pTables->m_decode_start_code_size = table_bits + 1;
               for (i = table_bits + 1; i <= max_code_size; i++)
               {
                  if (num_codes[i])
                  {
                     pTables->m_decode_start_code_size = i;
                     break;
                  }
               }
            }
         }

         // sentinels
         pTables->m_max_codes[cMaxExpectedCodeSize] = UINT_MAX;
         pTables->m_val_ptrs[cMaxExpectedCodeSize] = 0xFFFFF;

         pTables->m_table_shift = 32 - pTables->m_table_bits;

         return true;
      }
               
   } // namespace prefix_codig

} // namespace lzham


