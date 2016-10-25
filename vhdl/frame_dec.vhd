-- Hi Emacs, this is -*- mode: vhdl; vhdl-basic-offset: 6 -*-
------------------------------------------------------------------------------------------------------------------------
--
-- Byte Stream Serialized N-bit Frame Decoder
--
-- 
--
-- Copyright (c) 2016 Javier Valcarce García, <javier.valcarce@gmail.com>
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
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.numeric_std.all;

------------------------------------------------------------------------------------------------------------------------
--
-- This block has an interface compatible with Avalon-ST.
--
-- This block process an input stream of encoded bytes, detects frame limits and outputs the decoded frame.
-- Inputs 10 encoded bytes and output a frame of 9 bytes.
--
--
entity frame_dec is
      generic (
            N : integer := 10);
      port (
            reset_n     : in  std_logic;
            clk         : in  std_logic;
            octet_data  : in  std_logic_vector(07 downto 00);
            octet_valid : in  std_logic;
            octet_ready : out std_logic;
            frame_data  : out std_logic_vector(61 downto 00);  -- 10 bytes * 7 bits/byte=70 bits -8 bits crc
            frame_valid : out std_logic;
            frame_ready : in  std_logic);
end frame_dec;



------------------------------------------------------------------------------------------------------------------------
architecture rtl of frame_dec is

      subtype PACK0 is natural range 69 downto 63;
      subtype PACK1 is natural range 62 downto 56;
      subtype PACK2 is natural range 55 downto 49;
      subtype PACK3 is natural range 48 downto 42;
      subtype PACK4 is natural range 41 downto 35;
      subtype PACK5 is natural range 34 downto 28;
      subtype PACK6 is natural range 27 downto 21;
      subtype PACK7 is natural range 20 downto 14;
      subtype PACK8 is natural range 13 downto 07;
      subtype PACK9 is natural range 06 downto 00;

      signal frame_int       : std_logic_vector(69 downto 00);  -- 70 bits
      signal frame_valid_int : std_logic;
      signal c               : integer range 0 to 9;


begin
      frame_data  <= frame_int(69 downto 08); -- Elimino el CRC de 8 bits (LSB).
      frame_valid <= frame_valid_int;

      p_asm : process(reset_n, clk)
      begin
            if reset_n = '0' then
                  c               <= 0;
                  frame_valid_int <= '0';
                  octet_ready <= '1';
                  --frame_int       <= (others => '0');  -- not strictly required TODO: Remove

            elsif rising_edge(clk) then

                  if frame_valid_int = '1' then
                        -- A decoded and CRC-validated frame is ready to be delivered
                        if frame_ready = '1' then
                              c               <= 0;
                              frame_valid_int <= '0';
                              octet_ready <= '1';
                        end if;
                  else
                        
                        frame_valid_int <= '0'; -- redundant
                        octet_ready     <= '1';
                        
                        -- If there is no input data to process then go
                        -- directly to the end of this process.
                        if octet_valid = '1' then
                              if octet_data(7) = '1' then
                                    -- It marked as the first byte of the frame (MSB is '1')
                                    c                <= 1;
                                    frame_int(PACK0) <= octet_data(6 downto 0);
                              else
                                    if c = 1 then
                                          frame_int(PACK1) <= octet_data(6 downto 0);
                                          c                <= 2;
                                    elsif c = 2 then
                                          frame_int(PACK2) <= octet_data(6 downto 0);
                                          c                <= 3;
                                    elsif c = 3 then
                                          frame_int(PACK3) <= octet_data(6 downto 0);
                                          c                <= 4;
                                    elsif c = 4 then
                                          frame_int(PACK4) <= octet_data(6 downto 0);
                                          c                <= 5;
                                    elsif c = 5 then
                                          frame_int(PACK5) <= octet_data(6 downto 0);
                                          c                <= 6;
                                    elsif c = 6 then
                                          frame_int(PACK6) <= octet_data(6 downto 0);
                                          c                <= 7;
                                    elsif c = 7 then
                                          frame_int(PACK7) <= octet_data(6 downto 0);
                                          c                <= 8;
                                    elsif c = 8 then
                                          frame_int(PACK8) <= octet_data(6 downto 0);
                                          c                <= 9;
                                    elsif c = 9 then
                                          frame_int(PACK9) <= octet_data(6 downto 0);
                                          c                <= 0;
                                          frame_valid_int  <= '1';
                                          octet_ready      <= '0';
                                    end if;
                              end if;
                        end if;
                  end if;
            end if;
      end process;

end rtl;
