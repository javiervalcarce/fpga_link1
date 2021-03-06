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

    type state_type is (IDLE, SEL, PING_RES0, PING_RES1, RD_REQ0, RD_REQ1, RD_RES0, RD_RES1, WR_REQ0, WR_REQ1, WR_RES0, WR_RES1);
    signal state : state_type;

    --APB internals 
    signal opcode_req : natural range 0 to 15;  -- 6 bits
    signal opcode_res : natural range 0 to 15;  -- 6 bits

    signal psel_int           : std_logic;
    signal penable_int        : std_logic;
    signal paddr_int          : std_logic_vector(23 downto 00);
    signal pwrite_int         : std_logic;
    signal rx_frame_ready_int : std_logic;

    signal ireg    : std_logic_vector(61 downto 00);
    signal oreg    : std_logic_vector(61 downto 00);
    signal ireg_wr : std_logic;
    signal oreg_wr : std_logic;



    signal readdata : std_logic_vector(31 downto 00);

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
    paddr   <= paddr_int;
    psel    <= psel_int;
    penable <= penable_int;
    pwrite  <= pwrite_int;

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
    opcode_req <= to_integer(unsigned(ireg(61 downto 56)));
    paddr_int  <= ireg(55 downto 32);
    pwdata     <= ireg(31 downto 00);

    ----------------------------------------------------------------------------------------------------------------------

    -- tx frame:
    -- read data from apb slave (32 bits). TODO
    oreg(61 downto 56) <= std_logic_vector(to_unsigned(opcode_res, 6));
    oreg(55 downto 32) <= paddr_int;

    -- registers data delivered by the apb peripheral at the end of the apb
    -- read transfer
    p_read : process(reset_n, clk)
    begin
        if reset_n = '0' then
            oreg(31 downto 00) <= (others => '0');
        elsif rising_edge(clk) then
            if psel_int = '1' and penable_int = '1' and pwrite_int = '0' and pready = '1' then
                oreg(31 downto 00) <= prdata;
            end if;
        end if;
    end process p_read;



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
                    if opcode_req = CODE_PING then
                        state <= PING_RES0;
                    elsif opcode_req = CODE_READ32 then
                        state <= RD_REQ0;
                    elsif opcode_req = CODE_WRITE32 then
                        state <= WR_REQ0;
                    else
                        -- return to a known state, error recovery.
                        state <= IDLE;
                    end if;

                when PING_RES0 =>
                    state <= PING_RES1;
                when PING_RES1 =>
                    if tx_frame_ready = '1' then
                        state <= IDLE;
                    end if;



                -- APB READ
                when RD_REQ0 =>
                    state <= RD_REQ1;
                when RD_REQ1 =>
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
                when WR_REQ0 =>
                    state <= WR_REQ1;
                when WR_REQ1 =>
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
        psel_int           <= '0';
        penable_int        <= '0';
        pwrite_int         <= '0';
        
        opcode_res         <= CODE_PING_ACK;

        case state is
            when IDLE => psel_int <= '0'; penable_int <= '0'; pwrite_int <= '0'; rx_frame_ready_int <= '1';
            when SEL  => psel_int <= '0'; penable_int <= '0'; pwrite_int <= '0'; rx_frame_ready_int <= '0';

            when PING_RES0 => oreg_wr        <= '1'; opcode_res <= CODE_PING_ACK;
            when PING_RES1 => tx_frame_valid <= '1';

            when RD_REQ0 => psel_int       <= '1'; penable_int <= '0'; pwrite_int <= '0'; rx_frame_ready_int <= '0';
            when RD_REQ1 => psel_int       <= '1'; penable_int <= '1'; pwrite_int <= '0'; rx_frame_ready_int <= '0';
            when RD_RES0 => oreg_wr        <= '1'; opcode_res <= CODE_READ32_ACK;
            when RD_RES1 => tx_frame_valid <= '1';

            when WR_REQ0 => psel_int       <= '1'; penable_int <= '0'; pwrite_int <= '1'; rx_frame_ready_int <= '0';
            when WR_REQ1 => psel_int       <= '1'; penable_int <= '1'; pwrite_int <= '1'; rx_frame_ready_int <= '0';
            when WR_RES0 => oreg_wr        <= '1'; opcode_res <= CODE_WRITE32_ACK;
            when WR_RES1 => tx_frame_valid <= '1';

        end case;
    end process;

end rtl;
