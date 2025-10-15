module main where

macro F
        x = 1
end

export proc _start(void): !
{
        let x: i32 = 0;

        F;

        exit;
}
