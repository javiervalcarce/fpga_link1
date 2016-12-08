-- Hi Emacs, this is -*- mode: vhdl; vhdl-basic-offset: 6 -*--
------------------------------------------------------------------------------------------------------------------------
-- Interface to four 7-seg display to show 16-bit numbers in hexadecimal
--
-- Javier Valcarce García, javier.valcarce@gmail.com
-- $Id$
------------------------------------------------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity interface_7segx4_apb is
      port (
            reset_n   : in  std_logic;
            clk       : in  std_logic;
            psel      : in  std_logic;
            penable   : in  std_logic;
            pwrite    : in  std_logic;
            paddr     : in  std_logic_vector(23 downto 00);
            pwdata    : in  std_logic_vector(31 downto 00);
            prdata_in : in  std_logic_vector(31 downto 00);
            pready_in : in  std_logic;
            prdata    : out std_logic_vector(31 downto 00);
            pready    : out std_logic;
            do        : out std_logic_vector(07 downto 00);
            an        : out std_logic_vector(03 downto 00));
end interface_7segx4_apb;


architecture rtl of interface_7segx4_apb is

      -- ioregs
      type ioregs_t is record
            value : std_logic_vector(15 downto 00);
      end record;

      signal r : ioregs_t;

      -- apb signals
      signal prdata_int   : std_logic_vector(31 downto 00);
      signal pready_int   : std_logic;
      signal address4byte : unsigned(23 downto 00);

      -- I/O Registers Addresses
      constant IOADDR_BASE  : natural := 0;
      constant IOADDR_VALUE : natural := IOADDR_BASE + 4;

begin

      -- APB interface
      -- Daisy chain of apb slaves
      prdata <= prdata_in when psel = '0' else prdata_int;
      pready <= pready_in when psel = '0' else pready_int;

      address4byte <= unsigned(paddr(23 downto 00));

      -- genarate pready_int signal
      -- IO WITHOUT WAIT STATES (assert ready with setup phase)
      p_ready : process (reset_n, clk)
      begin
            if reset_n = '0' then
                  pready_int <= '0';
            elsif rising_edge(clk) then
                  if psel = '1' then
                        -- APB ACCESS Phase
                        pready_int <= '1';
                  else
                        -- APB SETUP Phase
                        pready_int <= '0';
                  end if;
            end if;
      end process p_ready;


      -- WR operation over i/o registers
      p_wr_ioregs : process (reset_n, clk)
      begin
            if reset_n = '0' then
                  -- DEFAULT VALUES AT RESET
                  r.value <= (others => '0');
            elsif rising_edge(clk) then

                  if psel = '1' and penable = '1' and pwrite = '1' then
                        case to_integer(address4byte) is
                              when IOADDR_VALUE =>
                                    r.value <= pwdata(15 downto 00);
                              when others =>
                                    r <= r;
                        end case;
                  end if;  --if psel = '1' and penable = '1' and pwrite = '1' then
            end if;  --elsif rising_edge(clk) then
      end process p_wr_ioregs;


      -- RD operation over i/o registers
      p_rd_ioregs : process(address4byte, r)
      begin
            case to_integer(address4byte) is
                  when IOADDR_VALUE => prdata_int <= X"00_00" & std_logic_vector(r.value);
                  when others       => prdata_int <= X"CA_CA_CA_CA";
            end case;
      end process p_rd_ioregs;


      -- output config signals stored in record r
      if7seg : entity work.interface_7segx4 port map (
            reset_n => reset_n,
            clk     => clk,
            di      => r.value,
            an      => an,
            do      => do);

end rtl;
