-- Hi Emacs, this is -*- mode: vhdl; vhdl-basic-offset: 6 -*-
------------------------------------------------------------------------------------------------------------------------
--
-- Byte Stream Serialized N-bit Frame Decoder
--
-- 
--
-- Copyright (c) 2016 Javier Valcarce Garcia, <javier.valcarce@gmail.com>
-- http://javiervalcarce.eu
--
------------------------------------------------------------------------------------------------------------------------
-- This program is free software: you can redistribute it and/or modify
-- it under the terms of the GNU Lesser General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Lesser General Public License for more details.
--
-- You should have received a copy of the GNU Lesser General Public License
-- along with this program. If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

------------------------------------------------------------------------------------------------------------------------
entity frame_dec_tb is
end frame_dec_tb;

------------------------------------------------------------------------------------------------------------------------
architecture sim of frame_dec_tb is

      constant SYSTEM_TCLK          : time := 0020.0 ns;  -- @50MHz
      
      
      shared variable END_SIMULATION : boolean := false;
      
      signal reset_n      : std_logic;
      signal clk          : std_logic;
      signal data         : std_logic_vector(7  downto 0);
      signal data_valid   : std_logic;
      signal data_ready   : std_logic;
      signal frame        : std_logic_vector(69 downto 0);
      signal frame_valid  : std_logic;
      signal frame_ready  : std_logic;

      
      --attribute INIT : string; 
      --attribute INIT of mux1_lut      : label is "E4FF";
      
begin

      
      -- system clock (@50MHz)
      p_system_clk : process
      begin
            if END_SIMULATION = false then
                  clk <= '0';
                  wait for SYSTEM_TCLK/2;
                  clk <= '1';
                  wait for SYSTEM_TCLK/2;
            else
                  wait;
            end if;       
      end process;

      reset_n <= '0' after 0.0 ns,      
                 '1' after 4.0 * SYSTEM_TCLK;

      p_input: process

            --constant kInputFrame : std_logic_vector(69 downto 00);
            
      begin

            -- simulo la entrada, byte a byte, de la siguiente trama codificada:

            -- opcode (6 bits) address (24 bits)    data (32 bits)  crc (8 bits)
            -- --------------- -------------------- --------------- ------------
            -- 0e              aa bb cc             01 02 03 04     00
            -- -----------------------------------------------------------------
            
            -- Coded:
            -- 9d 2a 5d 73 0a 56 33 5e 10 00
            
            data_valid <= '0';
            data <= X"00";
            frame_ready <= '0';
            
            wait until rising_edge(reset_n);

            assert (frame_valid = '0') report "frame_valid <> 0" severity error;

            wait until rising_edge(clk);
            data_valid <= '1';

            

            data <= X"9d";
            wait until rising_edge(clk);
            assert (frame_valid = '0') report "frame_valid <> 0" severity error;
            
            data <= X"2a";
            wait until rising_edge(clk);
            assert (frame_valid = '0') report "frame_valid <> 0" severity error;
            
            data <= X"5d";
            wait until rising_edge(clk);
            assert (frame_valid = '0') report "frame_valid <> 0" severity error;

            
            data <= X"73";
            wait until rising_edge(clk);
            assert (frame_valid = '0') report "frame_valid <> 0" severity error;

            
            data <= X"0a";
            wait until rising_edge(clk);
            assert (frame_valid = '0') report "frame_valid <> 0" severity error;

            
            data <= X"56";
            wait until rising_edge(clk);
            assert (frame_valid = '0') report "frame_valid <> 0" severity error;

            
            data <= X"33";
            wait until rising_edge(clk);
            assert (frame_valid = '0') report "frame_valid <> 0" severity error;

            
            data <= X"5e";
            wait until rising_edge(clk);
            assert (frame_valid = '0') report "frame_valid <> 0" severity error;
            
            data <= X"10";
            wait until rising_edge(clk);
            assert (frame_valid = '0') report "frame_valid <> 0" severity error;

            
            data <= X"00";
            wait until rising_edge(clk);

            data_valid <= '0';
            
            -- give some time more to process the last byte
            wait until rising_edge(clk);

            
            wait until rising_edge(clk);
            wait until rising_edge(clk);
            wait until rising_edge(clk);
            wait until rising_edge(clk);
            wait until rising_edge(clk);

            

            assert (frame_valid = '1') report "frame_valid <> 1" severity error;

            frame_ready <= '1';
            wait until rising_edge(clk);
            
            frame_ready <= '0';
            wait until rising_edge(clk);
            wait until rising_edge(clk);
            wait until rising_edge(clk);
            wait until rising_edge(clk);
            
            END_SIMULATION := true;
      end process;
      

      -- entity instantiation (VHDL'93)
      uut: entity work.frame_dec port map (
            reset_n     => reset_n,
            clk         => clk,
            data        => data,
            data_valid  => data_valid,
            data_ready  => data_ready,
            frame       => frame,
            frame_valid => frame_valid,
            frame_ready => frame_ready
      );

      

end sim;
