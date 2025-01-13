#include <gtest/gtest.h>
#include "../code/BMPfile.h"

TEST(BMPfileTest, ConstructorStandartd){
    BMPfile file(2,2);
    file.saveBmp("Resource/CS.bmp");
    EXPECT_EQ(0,std::system("diff Resource/CS.bmp Resource/test1.bmp"));
    std::system("rm -rf Resource/CS.bmp");
}

TEST(BMPfileTest, ConstructorWithFile){
    BMPfile file("Resource/Qr.bmp");
    file.saveBmp("Resource/CWF.bmp");
    EXPECT_EQ(0,std::system("diff Resource/CWF.bmp Resource/Qr.bmp"));
    std::system("rm -rf Resource/CWF.bmp");
}

TEST(BMPfileTest, Setter){
    BMPfile file(2,2);
    file.setPixel(0,0, false);
    file.setPixel(1,1, false);
    file.saveBmp("Resource/setter.bmp");
    EXPECT_EQ(0,std::system("diff Resource/setter.bmp Resource/test2.bmp"));
    std::system("rm -rf Resource/setter.bmp");
}

TEST(BMPfileTest, Getter){
    BMPfile file(2,2);
    EXPECT_EQ(true,file.getPixel(0,0));
    file.setPixel(0,0, false);
    EXPECT_EQ(false,file.getPixel(0,0));
    EXPECT_EQ(true,file.getPixel(0,1));
    file.setPixel(0,1, false);
    EXPECT_EQ(false,file.getPixel(0,1));
    EXPECT_EQ(true,file.getPixel(1,0));
    file.setPixel(1,0, false);
    EXPECT_EQ(false,file.getPixel(1,0));
    EXPECT_EQ(true,file.getPixel(1,1));
    file.setPixel(1,1, false);
    EXPECT_EQ(false,file.getPixel(1,1));
    file.setPixel(1,1,true);
    EXPECT_EQ(true,file.getPixel(1,1));
    file.setPixel(1,0,true);
    EXPECT_EQ(true,file.getPixel(1,0));
    file.setPixel(0,0,true);
    EXPECT_EQ(true,file.getPixel(0,0));
    file.setPixel(0,1,true);
    EXPECT_EQ(true,file.getPixel(0,1));
    EXPECT_EQ(2,file.getHeight());
    EXPECT_EQ(2,file.getWidth());
    file={1,1};
    EXPECT_TRUE(file.getPixel(0,0));
    file.setPixel(0,0,false);
    EXPECT_FALSE(file.getPixel(0,0));
    file.setPixel(0,0,true);
    EXPECT_EQ(1,file.getHeight());
    EXPECT_EQ(1,file.getWidth());
    //file.saveBmp("Resource/Hi.bmp");
    file={9,20};
    EXPECT_EQ(20,file.getHeight());
    EXPECT_EQ(9,file.getWidth());
    EXPECT_TRUE(file.getPixel(2,4));
    file.setPixel(2,4,false);
    EXPECT_FALSE(file.getPixel(2,4));
    file.setPixel(2,4,true);
    EXPECT_TRUE(file.getPixel(2,4));
}

bool fooForROF(const BMPfile& file){
    if(file.getHeight()==2 and file.getWidth()==2)
        return !file.getPixel(0,0) and
        file.getPixel(0,1) and
        file.getPixel(1,0) and
        !file.getPixel(1,1);
    return false;
}

TEST(BMPfileTest, RulesOfFive){
    BMPfile file(2,2);
    file.setPixel(0,0, false);
    file.setPixel(1,1, false);
    EXPECT_TRUE(fooForROF(file));
    auto b=file;
    EXPECT_TRUE(fooForROF(b));
    auto c(file);
    EXPECT_TRUE(fooForROF(c));
    auto d = std::move(b);
    EXPECT_TRUE(fooForROF(d));
    auto e(std::move(c));
    EXPECT_TRUE(fooForROF(e));
}

TEST(BMPfileTest, Resize){
    BMPfile file(2,2);
    file.setPixel(0,0, false);
    file.setPixel(1,1, false);
    //file.saveBmp("Resource/Hi.bmp");
    EXPECT_EQ(2,file.getHeight());
    EXPECT_EQ(2,file.getWidth());
    EXPECT_FALSE(file.getPixel(0,0));
    EXPECT_TRUE(file.getPixel(1,0));
    EXPECT_TRUE(file.getPixel(0,1));
    EXPECT_FALSE(file.getPixel(1,1));
    file.resize(5,3);
    //file.saveBmp("Resource/Hi2.bmp");
    EXPECT_EQ(5,file.getWidth());
    EXPECT_EQ(3,file.getHeight());
    EXPECT_FALSE(file.getPixel(0,0));
    EXPECT_TRUE(file.getPixel(1,0));
    EXPECT_TRUE(file.getPixel(0,1));
    EXPECT_FALSE(file.getPixel(1,1));
    EXPECT_TRUE(file.getPixel(0,2));
    EXPECT_TRUE(file.getPixel(1,2));
    EXPECT_TRUE(file.getPixel(0,3));
    EXPECT_TRUE(file.getPixel(1,3));
    EXPECT_TRUE(file.getPixel(0,4));
    EXPECT_TRUE(file.getPixel(1,4));
    file={4,4};
    file.setPixel(0,0,false);
    file.setPixel(0,2,false);
    file.setPixel(1,1,false);
    file.setPixel(1,3,false);
    file.setPixel(2,0,false);
    file.setPixel(2,2,false);
    file.setPixel(3,1,false);
    file.setPixel(3,3,false);
    //file.saveBmp("Resource/Hi3.bmp");
    file.resize(2,2);
    //file.saveBmp("Resource/Hi4.bmp");
    EXPECT_EQ(2,file.getWidth());
    EXPECT_EQ(2,file.getHeight());
    EXPECT_FALSE(file.getPixel(0,0));
    EXPECT_TRUE(file.getPixel(0,1));
    EXPECT_TRUE(file.getPixel(1,0));
    EXPECT_FALSE(file.getPixel(1,1));
}

TEST(BMPfileTest, Thrtows){
    EXPECT_ANY_THROW(BMPfile("hehe"));
    EXPECT_ANY_THROW(BMPfile(0,0));
    EXPECT_ANY_THROW(BMPfile(1,0));
    EXPECT_ANY_THROW(BMPfile(0,1));
    BMPfile file(2,2);
    EXPECT_ANY_THROW(file.setPixel(2,2,false));
    EXPECT_ANY_THROW(file.setPixel(2,2,true));
    EXPECT_ANY_THROW(file.setPixel(20,2,false));
    EXPECT_ANY_THROW(file.setPixel(2,20,true));
    EXPECT_ANY_THROW(file.getPixel(2,2));
    EXPECT_ANY_THROW(file.getPixel(2,2));
    EXPECT_ANY_THROW(file.getPixel(20,2));
    EXPECT_ANY_THROW(file.getPixel(2,20));
    EXPECT_ANY_THROW(file.resize(0,0));
    EXPECT_ANY_THROW(file.resize(1,0));
    EXPECT_ANY_THROW(file.resize(0,1));
}

TEST(BMPfileTest, SymBmp_1){
    BMPfile file(2,2);
    file.setPixel(0,0, false);
    file.setPixel(1,1, false);
    file.symResize(4,4);
    EXPECT_TRUE(file.getPixel(0,0));
    EXPECT_TRUE(file.getPixel(0,1));
    EXPECT_TRUE(file.getPixel(0,2));
    EXPECT_TRUE(file.getPixel(0,3));
    EXPECT_TRUE(file.getPixel(0,0));
    EXPECT_TRUE(file.getPixel(1,0));
    EXPECT_TRUE(file.getPixel(2,0));
    EXPECT_TRUE(file.getPixel(3,0));
    EXPECT_TRUE(file.getPixel(3,0));
    EXPECT_TRUE(file.getPixel(3,1));
    EXPECT_TRUE(file.getPixel(3,2));
    EXPECT_TRUE(file.getPixel(3,3));
    EXPECT_TRUE(file.getPixel(0,3));
    EXPECT_TRUE(file.getPixel(1,3));
    EXPECT_TRUE(file.getPixel(2,3));
    EXPECT_TRUE(file.getPixel(3,3));
    EXPECT_TRUE(file.getPixel(1,2));
    EXPECT_TRUE(file.getPixel(2,1));
    EXPECT_FALSE(file.getPixel(1,1));
    EXPECT_FALSE(file.getPixel(2,2));
}

TEST(BMPfileTest, SymBmp_2){
     BMPfile file{2,2};
    file.setPixel(0,0,false);
    file.setPixel(1,1,false);
    file.symResize(3,3);
    EXPECT_TRUE(file.getPixel(0,1));
    EXPECT_TRUE(file.getPixel(0,2));
    EXPECT_TRUE(file.getPixel(1,0));
    EXPECT_TRUE(file.getPixel(1,2));
    EXPECT_TRUE(file.getPixel(2,0));
    EXPECT_TRUE(file.getPixel(2,1));
    EXPECT_TRUE(file.getPixel(2,2));
    EXPECT_FALSE(file.getPixel(0,0));
    EXPECT_FALSE(file.getPixel(1,1));
}

TEST(BMPfileTest, SymBmp_3){
    BMPfile file{2,2};
    file.setPixel(0,0,false);
    file.setPixel(1,1,false);
    file.symResize(3,4);
    EXPECT_TRUE(file.getPixel(0,0));
    EXPECT_TRUE(file.getPixel(0,1));
    EXPECT_TRUE(file.getPixel(0,2));
    EXPECT_TRUE(file.getPixel(1,1));
    EXPECT_TRUE(file.getPixel(1,2));
    EXPECT_TRUE(file.getPixel(2,0));
    EXPECT_TRUE(file.getPixel(2,2));
    EXPECT_TRUE(file.getPixel(3,0));
    EXPECT_TRUE(file.getPixel(3,1));
    EXPECT_TRUE(file.getPixel(3,2));
    EXPECT_FALSE(file.getPixel(1,0));
    EXPECT_FALSE(file.getPixel(2,1));
}

TEST(BMPfileTest, SymBmp_4){
    BMPfile file{2,2};
    file.setPixel(0,0,false);
    file.setPixel(1,1,false);
    file.symResize(4,3);
    EXPECT_TRUE(file.getPixel(0,0));
    EXPECT_TRUE(file.getPixel(1,0));
    EXPECT_TRUE(file.getPixel(2,0));
    EXPECT_TRUE(file.getPixel(1,1));
    EXPECT_TRUE(file.getPixel(2,1));
    EXPECT_TRUE(file.getPixel(0,2));
    EXPECT_TRUE(file.getPixel(2,2));
    EXPECT_TRUE(file.getPixel(0,3));
    EXPECT_TRUE(file.getPixel(1,3));
    EXPECT_TRUE(file.getPixel(2,3));
    EXPECT_FALSE(file.getPixel(0,1));
    EXPECT_FALSE(file.getPixel(1,2));
}

TEST(BMPfileTest, SymBmp_5){
    BMPfile file{4,4};
    file.setPixel(1,1,false);
    file.setPixel(2,2,false);
    file.symResize(2,2);
    EXPECT_TRUE(file.getPixel(0,1));
    EXPECT_TRUE(file.getPixel(1,0));
    EXPECT_FALSE(file.getPixel(0,0));
    EXPECT_FALSE(file.getPixel(1,1));
}

TEST(BMPfileTest, SymBmp_6){
    BMPfile file(2,4);
    file.setPixel(1,0,false);
    file.setPixel(2,1,false);
    file.symResize(2,2);
    EXPECT_FALSE(file.getPixel(0,0));
    EXPECT_FALSE(file.getPixel(1,1));
    EXPECT_TRUE(file.getPixel(0,1));
    EXPECT_TRUE(file.getPixel(1,0));
}