-- Hi Emacs, this is -*- mode: vhdl; vhdl-basic-offset: 6 -*-
------------------------------------------------------------------------------------------------------------------------
--
-- Byte Stream Deserializer 70-bit Frame Decoder
--
-- Created:           2016-08-01
-- Last Modification: 2016-08-01
--
-- Copyright (c) 2016 Javier Valcarce García, <javier.valcarce@gmail.com>
-- http://javiervalcarce.eu
--
------------------------------------------------------------------------------------------------------------------------
-- This program is free software: you can redistribute it and/or modify
-- it under the terms of the GNU Lesser General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.

-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Lesser General Public License for more details.
--
-- You should have received a copy of the GNU Lesser General Public License
-- along with this program. If not, see <http://www.gnu.org/licenses/>.

------------------------------------------------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

--
-- This entity has an Avalon-ST interface.
-- Ready Latency: zero
--
-- The signals _valid and _ready does not need explanation. See Avalon-ST
-- interface specification.
--
-- This block accepts a 62 bits frame in bus |frame_data| and output a 70 bits
-- (8 bits crc appended) frame byte by byte in bus |octet_data|.
entity frame_enc is
      port (
            reset_n     : in  std_logic;
            clk         : in  std_logic;
            --
            octet_data  : out std_logic_vector(07 downto 00);
            octet_valid : out std_logic;
            octet_ready : in  std_logic;
            --
            frame_data  : in  std_logic_vector(61 downto 00);
            frame_valid : in  std_logic;
            frame_ready : out std_logic);
end frame_enc;

------------------------------------------------------------------------------------------------------------------------
architecture rtl of frame_enc is

      subtype PACK0 is natural range 69 downto 63;  -- 7 bits.
      subtype PACK1 is natural range 62 downto 56;  -- 7 bits.
      subtype PACK2 is natural range 55 downto 49;  -- 7 bits.
      subtype PACK3 is natural range 48 downto 42;  -- etc
      subtype PACK4 is natural range 41 downto 35;
      subtype PACK5 is natural range 34 downto 28;
      subtype PACK6 is natural range 27 downto 21;
      subtype PACK7 is natural range 20 downto 14;
      subtype PACK8 is natural range 13 downto 07;
      subtype PACK9 is natural range 06 downto 00;

      signal start      : std_logic;
      signal frame_ireg : std_logic_vector(61 downto 00);
      signal frame_wr   : std_logic;
      signal frame_wcrc : std_logic_vector(69 downto 00);
      

      signal frame_ready_int : std_logic;
      signal octet_valid_int : std_logic;

      -- MSB (Most Significant Bit) set to '1' to mark the first byte, all others set to '0'

      -- mux1 (10 channels of 7 bits each channel)
      signal mux1_data : std_logic_vector(07 downto 00);
      signal mux1_sel  : natural range 10 downto 0;

      -- output register
      signal octet_en : std_logic;
      signal octet_q  : std_logic_vector(07 downto 00);

      signal crc_start : std_logic;
      signal crc_crc   : std_logic_vector(07 downto 00);
      
begin

      start      <= frame_valid and frame_ready_int;
      frame_wr   <= start;
      frame_wcrc <= frame_ireg & crc_crc;
      
      octet_data <= octet_q;

      octet_valid <= octet_valid_int;
      frame_ready <= frame_ready_int;

      -- Input register
      p_ireg : process(reset_n, clk)
      begin
            if reset_n = '0' then
                  frame_ireg <= (others => '0');
            elsif rising_edge(clk) then
                  if frame_wr = '1' then
                        frame_ireg <= frame_data;
                  end if;
            end if;
      end process;

      -- Multiplexer #1 (7-bits chanels)
      p_mux7 : process(frame_wcrc, mux1_sel)
      begin
            case mux1_sel is
                  when 0      => mux1_data <= "1" & frame_wcrc(PACK0);
                  when 1      => mux1_data <= "0" & frame_wcrc(PACK1);
                  when 2      => mux1_data <= "0" & frame_wcrc(PACK2);
                  when 3      => mux1_data <= "0" & frame_wcrc(PACK3);
                  when 4      => mux1_data <= "0" & frame_wcrc(PACK4);
                  when 5      => mux1_data <= "0" & frame_wcrc(PACK5);
                  when 6      => mux1_data <= "0" & frame_wcrc(PACK6);
                  when 7      => mux1_data <= "0" & frame_wcrc(PACK7);
                  when 8      => mux1_data <= "0" & frame_wcrc(PACK8);
                  when 9      => mux1_data <= "0" & frame_wcrc(PACK9);  --crc
                  when others => mux1_data <= "--------";
            end case;
      end process;


      -- Output register
      p_oreg : process(reset_n, clk)
      begin
            if reset_n = '0' then
                  octet_q <= (others => '0');
            elsif rising_edge(clk) then
                  if octet_en = '1' then
                        octet_q <= mux1_data;
                  end if;
            end if;
      end process;



      -------------------------------------------------------------------------------
      -- CRC calculator subsystem
      -------------------------------------------------------------------------------
      CRC : block

            signal mux2_sel  : natural range 7 downto 0;
            signal crc_data  : std_logic_vector(07 downto 00);
            signal crc_run   : std_logic;
      begin

            p_ctl : process(reset_n, clk)
            begin
                  if reset_n = '0' then
                        crc_run  <= '0';
                        mux2_sel <= 0;
                  elsif rising_edge(clk) then
                        if crc_start = '1' then
                              crc_run <= '1';
                        end if;

                        if crc_run = '1' then
                              if mux2_sel /= 7 then
                                    mux2_sel <= mux2_sel + 1;
                              end if;
                              if mux2_sel = 7 then
                                    crc_run <= '0';
                              end if;
                        end if;

                  end if;
            end process;


            -- Multiplexer #2 (8-bits chanels)
            p_mux : process(frame_ireg, mux2_sel)
            begin
                  case mux2_sel is
                        when 0 => crc_data <= "00" & frame_ireg(61 downto 56);
                        when 1 => crc_data <= frame_ireg(55 downto 48);
                        when 2 => crc_data <= frame_ireg(47 downto 40);
                        when 3 => crc_data <= frame_ireg(39 downto 32);
                        when 4 => crc_data <= frame_ireg(31 downto 24);
                        when 5 => crc_data <= frame_ireg(23 downto 16);
                        when 6 => crc_data <= frame_ireg(15 downto 08);
                        when 7 => crc_data <= frame_ireg(07 downto 00);
                  end case;
            end process;

            p_crc : entity work.crc8d8 port map (
                  clk  => clk,
                  rs   => crc_start,
                  data => crc_data,
                  en   => crc_run,
                  crc  => crc_crc
                  );

      end block CRC;
      -------------------------------------------------------------------------------



      -------------------------------------------------------------------------------
      -- Control Unit
      -------------------------------------------------------------------------------
      CTL : block
            type state_type is (e0, e1, e2, e3, e4);
            signal state : state_type;
            signal op    : std_logic_vector(2 downto 0);

            signal ctr_en  : std_logic;
            signal ctr_rs  : std_logic;
            signal ctr_end : std_logic;
            signal ctr_val : natural range 10 downto 0;

      begin
            ctr_end  <= '1' when ctr_val = 10 else '0';
            mux1_sel <= ctr_val;

            p_ctr : process(reset_n, clk)
                  variable c : natural range 0 to 10;
            begin
                  if rising_edge(clk) then
                        if ctr_rs = '1' then
                              ctr_val <= 0;
                        else
                              if ctr_en = '1' then
                                    ctr_val <= ctr_val + 1;
                              end if;
                        end if;
                  end if;
            end process p_ctr;

            -- 2 procesos para separar la parte secuencial de la combinacional, de
            -- esta forma las salidas no son registros ("registered outputs") y por
            -- tanto no hay un ciclo de reloj de espera
            p_ctl : process(reset_n, clk)
            begin
                  if reset_n = '0' then
                        state <= e0;
                  elsif rising_edge(clk) then

                        case (state) is
                              when e0 =>
                                    if start = '1' then
                                          state <= e1;
                                    end if;
                              when e1 =>
                                    state <= e2;
                              when e2 =>
                                    state <= e3;
                              when e3 =>
                                    if octet_ready = '1' then
                                          state <= e4;
                                    end if;
                              when e4 =>
                                    if ctr_end = '1' then
                                          state <= e0;
                                    else
                                          state <= e2;
                                    end if;
                        end case;
                  end if;
            end process p_ctl;

            p_dec : process(state)
            begin
                  frame_ready_int <= '0';
                  octet_valid_int <= '0';
                  ctr_rs          <= '0';
                  ctr_en          <= '0';
                  octet_en        <= '0';
                  crc_start       <= '0';

                  case state is
                        when e0 => frame_ready_int <= '1'; ctr_rs <= '1';
                        when e1 => crc_start       <= '1';
                        when e2 => ctr_en          <= '1'; octet_en <= '1';
                        when e3 => octet_valid_int <= '1';
                        when e4 => null;
                  end case;
            end process p_dec;

      end block CTL;

end rtl;
