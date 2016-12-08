library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.numeric_std.all;


entity apb_master is
  port(
    clk            : in  std_logic;
    reset_n        : in  std_logic;
    -- rx frame
    rx_frame_data  : in  std_logic_vector(61 downto 00);
    rx_frame_valid : in  std_logic;
    rx_frame_ready : out std_logic;
    -- tx frame
    tx_frame_data  : out std_logic_vector(61 downto 00);
    tx_frame_valid : out std_logic;
    tx_frame_ready : in  std_logic;
    -- APB Slaves 
    psel           : out std_logic;
    pwrite         : out std_logic;
    penable        : out std_logic;
    paddr          : out std_logic_vector(23 downto 00);
    pwdata         : out std_logic_vector(31 downto 00);
    prdata         : in  std_logic_vector(31 downto 00);
    pready         : in  std_logic
    );

end apb_master;




architecture rtl of apb_master is

  type state_type is (IDLE, SEL, RD0, RD1, RD_RES0, RD_RES1, WR0, WR1, WR_RES0, WR_RES1);
  signal state : state_type;

  --APB internals 
  signal opcode             : natural range 0 to 15;  -- 6 bits
  signal paddr_int          : std_logic_vector(23 downto 00);
  signal rx_frame_ready_int : std_logic;
  signal ireg               : std_logic_vector(61 downto 00);
  signal oreg               : std_logic_vector(61 downto 00);
  signal ireg_wr            : std_logic;
  signal oreg_wr            : std_logic;

  signal res_wr : std_logic;

  constant CODE_PING         : natural := 1;
  constant CODE_READ32       : natural := 2;
  constant CODE_WRITE32      : natural := 3;
  constant CODE_PING_ACK     : natural := 4;
  constant CODE_READ32_ACK   : natural := 5;
  constant CODE_READ32_NACK  : natural := 6;
  constant CODE_WRITE32_ACK  : natural := 7;
  constant CODE_WRITE32_NACK : natural := 8;
  constant CODE_INTERRUPT    : natural := 9;

begin

  -- dummy signals
  paddr          <= paddr_int;
  rx_frame_ready <= rx_frame_ready_int;

  ----------------------------------------------------------------------------------------------------------------------
  -- fsm start!
  ireg_wr <= rx_frame_valid and rx_frame_ready_int;

  p_ireg : process(reset_n, clk)
  begin
    if reset_n = '0' then
      ireg <= (others => '0');
    elsif rising_edge(clk) then
      if ireg_wr = '1' then
        ireg <= rx_frame_data;
      end if;
    end if;
  end process p_ireg;

  -- received frame: opcode (6 bits), address (24 bits) and data (32 bits)
  opcode    <= to_integer(unsigned(ireg(61 downto 56)));
  paddr_int <= ireg(55 downto 32);
  pwdata    <= ireg(31 downto 00);

  ----------------------------------------------------------------------------------------------------------------------

  -- tx frame:
  -- read data from apb slave (32 bits). TODO
  oreg(61 downto 56) <= std_logic_vector(to_unsigned(CODE_READ32_ACK, 6)) when res_wr = '0' else
                        std_logic_vector(to_unsigned(CODE_WRITE32_ACK, 6));


  oreg(55 downto 32) <= paddr_int;
  oreg(31 downto 00) <= prdata;

  p_oreg : process(reset_n, clk)
  begin
    if reset_n = '0' then
      tx_frame_data <= (others => '0');
    elsif rising_edge(clk) then
      if oreg_wr = '1' then
        tx_frame_data <= oreg;
      end if;
    end if;
  end process p_oreg;

  ----------------------------------------------------------------------------------------------------------------------


  -- Graph transitions
  process (reset_n, clk)
  begin
    if reset_n = '0' then
      state <= IDLE;

    elsif rising_edge(clk) then
      case (state) is
        when IDLE =>
          if ireg_wr = '1' then
            state <= SEL;
          end if;
        when SEL =>
          -- TODO: Errors in APB transacctions
          if opcode = CODE_READ32 then
            state <= RD0;
          elsif opcode = CODE_WRITE32 then
            state <= WR0;
          end if;

        -- APB READ
        when RD0 =>
          state <= RD1;
        when RD1 =>
          if pready = '1' then
            state <= RD_RES0;
          end if;
        when RD_RES0 =>
          state <= RD_RES1;
        when RD_RES1 =>
          if tx_frame_ready = '1' then
            state <= IDLE;
          end if;

        -- APB WRITE
        when WR0 =>
          state <= WR1;
        when WR1 =>
          if pready = '1' then
            state <= WR_RES0;
          end if;
        when WR_RES0 =>
          state <= WR_RES1;
        when WR_RES1 =>
          if tx_frame_ready = '1' then
            state <= IDLE;
          end if;

      -- TODO
      end case;
    end if;
  end process;


  -- control signals for data path
  process (state)
  begin
    oreg_wr            <= '0';
    rx_frame_ready_int <= '0';
    tx_frame_valid     <= '0';
    psel               <= '0';
    penable            <= '0';
    pwrite             <= '0';
    res_wr             <= '0';

    case state is
      when IDLE => psel <= '0'; penable <= '0'; pwrite <= '0'; rx_frame_ready_int <= '1';
      when SEL  => psel <= '0'; penable <= '0'; pwrite <= '0'; rx_frame_ready_int <= '0';

      when RD0     => psel           <= '1'; penable <= '0'; pwrite <= '0'; rx_frame_ready_int <= '0';
      when RD1     => psel           <= '1'; penable <= '1'; pwrite <= '0'; rx_frame_ready_int <= '0';
      when RD_RES0 => oreg_wr        <= '1'; res_wr <= '0';
      when RD_RES1 => tx_frame_valid <= '1';

      when WR0     => psel           <= '1'; penable <= '0'; pwrite <= '1'; rx_frame_ready_int <= '0';
      when WR1     => psel           <= '1'; penable <= '1'; pwrite <= '1'; rx_frame_ready_int <= '0';
      when WR_RES0 => oreg_wr        <= '1'; res_wr <= '1';
      when WR_RES1 => tx_frame_valid <= '1';

    end case;
  end process;





end rtl;
