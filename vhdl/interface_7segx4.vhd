-- Hi Emacs, this is -*- mode: vhdl; vhdl-basic-offset: 6 -*--
--------------------------------------------------------------------------------
-- Interface to four 7-seg display to show 16-bit numbers in hexadecimal
--
-- Javier Valcarce García, javier.valcarce@gmail.com
-- $Id$
--------------------------------------------------------------------------------


library ieee;

use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-------------------------------------------------------------------------------
-------------------------------------------------------------------------------

entity interface_7segx4 is
      port (
            reset_n  : in  std_logic;
            clk      : in  std_logic;
            di       : in  std_logic_vector(15 downto 0);
            an       : out std_logic_vector(3 downto 0);  -- enable visualizers
            do       : out std_logic_vector(7 downto 0)  -- data for each 7-seg display
            );
end interface_7segx4;


-------------------------------------------------------------------------------
-------------------------------------------------------------------------------

architecture rtl of interface_7segx4 is

      signal muxo : std_logic_vector(3 downto 0);  -- mux output
      signal msel : std_logic_vector(1 downto 0);  -- mux sel  
      --signal divo : std_logic_vector(15 downto 0);  -- clk divider
      --signal slow : std_logic;                      -- slow clk

begin

      -- 2-bit counter for activate one 7seg display each time (time multiplex via
      -- decoder)

      -- Clock divider, from 50MHz to aprox. 1kHz
      -- THE CLOCK SIGNAL THAT DRIVES THIS COUNTER MUST BE SLOW ENOUGHT TO LET THE
      -- DIODES BRIGH CONSIDERING THE SWITCH SPEED OF THIS DEVICES. f = 1kHz is ok.
      process (reset_n, clk)
            variable c : integer range 0 to 50000;
      begin
            if reset_n = '0' then
                  c    := 0;
                  msel <= "00";

            elsif rising_edge(clk) then
                  c := c + 1;
                  if c = 0 then
                        msel <= msel + 1;
                  end if;
            end if;
      end process;

      -- Decoder, enable signal is active low
      an <= "1110" when msel = "00" else
            "1101" when msel = "01" else
            "1011" when msel = "10" else
            "0111";                     -- "11"

      -- Mux
      muxo <= di(15 downto 12) when msel = "11" else
              di(11 downto 08) when msel = "10" else
              di(07 downto 04) when msel = "01" else
              di(03 downto 00);         -- "00"

      -- BCD to 7seg decoder
      -- DP G F E D C B A
      do <= "11000000" when muxo = "0000" else  -- 0
            "11111001" when muxo = "0001" else  -- 1
            "10100100" when muxo = "0010" else  -- 2
            "10110000" when muxo = "0011" else  -- 3
            "10011001" when muxo = "0100" else  -- 4
            "10010010" when muxo = "0101" else  -- 5        
            "10000010" when muxo = "0110" else  -- 6
            "11111000" when muxo = "0111" else  -- 7
            "10000000" when muxo = "1000" else  -- 8
            "10010000" when muxo = "1001" else  -- 9
            "10001000" when muxo = "1010" else  -- A
            "10000011" when muxo = "1011" else  -- B
            "11000110" when muxo = "1100" else  -- C
            "10100001" when muxo = "1101" else  -- D
            "10000110" when muxo = "1110" else  -- E
            "10001110" when muxo = "1111";      -- F

end rtl;
